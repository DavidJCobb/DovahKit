#include "scene.h"
#include "./frame_in_flight.h"
#include "./surface_renderer.h"
//
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
//
#include "./config/is_righthanded.h"
#include "./config/scene_limits.h"
#include "./config/shadow_maps.h"
#include "./config/use_inverted_depth.h"
#include "./helpers/cubemap_helpers.h"
#include "./helpers/extract_frustum_normals.h"
#include "./data/DKVulkanCameraUpdate.h"
#include "./data/camera_coordinate_change.h"

// for NIF support
#include "nif/file.h"
#include "nif/blocks/NiNode.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/blocks/NiGeometryData.h"
#include "./rendered_nif.h"

#include <glm/gtx/matrix_decompose.hpp> // for debugging

namespace {
   constexpr float draw_distance_near = 0.1F;
   constexpr float draw_distance_far  = 1000.0F;

   constexpr float shadow_draw_distance = 7000; // lateral draw distance for the sun's shadows
   constexpr float shadow_draw_depth    = 7000; // depth   draw distance for the sun's shadows
}

namespace vulkanDK {
   scene_camera::scene_camera(scene& o) {
      this->_callback = {
         .context = &o,
         .functor = [](void* context, bool translated, bool rotated) {
            auto& owner = *(scene*)context;
            owner.global_state.view       = owner.camera.view_matrix();
            owner.global_state.camera_pos = owner.camera.position();
            if (translated) {
               owner.mark_light_shadows_dirty();
               owner.update_sun_shadows();
            }
         }
      };
   }

   scene::scene() : camera(*this) {
      this->global_state.ambient_light_color = { 0.1, 0.1, 0.1 };
      this->camera.set_coordinates(
         { 2.0F, 2.0F, 2.0F },
         { glm::radians(-45.0F), 0, glm::radians(-45.0F) }
      );
   }

   void scene::update_projection(VkExtent2D render_area) {
      //
      // NOTE: If we change any of the math here, be sure to update `scene::get_inverse_projection_matrix` 
      // as well, to keep it in synch with what's done here!
      //
      float aspect = 1.0F;
      if (render_area.height != 0.0)
         aspect = (float)render_area.width / (float)render_area.height;
      //
      this->last_known_view_info.bounds = render_area;
      this->last_known_view_info.aspect = aspect;
      //
      {
         auto& proj = this->global_state.proj;
         if constexpr (config::use_inverted_depth) {
            //
            // Use reversed depth and infinite far plane:
            //
            auto y_scale = 1.0F / tan(glm::radians(this->config.vertical_fov_degrees) / 2.0F);
            auto x_scale = y_scale / aspect;
            proj = glm::mat4(
               x_scale, 0,       0,  0,
               0,       y_scale, 0,  0,
               0,       0,       0, -1,
               0,       0,       draw_distance_near, 0
            );
            //
            // sources: <https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/>
            //          <https://dev.theomader.com/depth-precision/>
            //
         } else {
            proj = glm::perspectiveRH_ZO(glm::radians(this->config.vertical_fov_degrees), aspect, draw_distance_near, draw_distance_far);
         }
         if constexpr (config::is_righthanded) {
            //
            // GLM was designed for OpenGL, which uses an inverted Y axis. We need to flip the 
            // Y-axis here. Do be aware, however, that this is a 3D flip; vertex order will 
            // change handedness (clockwise/counterclockwise), which will affect what Vulkan 
            // considers a "backface" versus a "frontface."
            //
            proj[1][1] *= -1;
         }
      }
      //
      this->update_sun_shadows();
   }
   glm::mat4 scene::get_inverse_projection_matrix(VkExtent2D render_area) const {
      if constexpr (config::use_inverted_depth) {
         // Used https://mathdf.com/mat/#expr=inv(A)&mats=A(1%2Ftan(f))%2F(w%2Fh)~0~0~0!0~1%2Ftan(f)~0~0!0~0~0~d!0~0~-1~0%5D
         // to figure out the math.

         float aspect = 1.0F;
         if (render_area.height != 0.0)
            aspect = (float)render_area.width / (float)render_area.height;

         auto inv_y_scale = tan(glm::radians(this->config.vertical_fov_degrees) / 2.0F);
         auto inv_x_scale = inv_y_scale * aspect;
         if constexpr (config::is_righthanded) {
            inv_y_scale *= -1;
         }
         //
         // NOTE: The glm::mat4 constructor that takes sixteen scalars takes them in 
         // column-major order -- so, the first column's X, Y, Z, and W; then the 
         // second column; then the third; then the fourth.
         //
         return glm::fmat4(
            inv_x_scale, 0, 0, 0,
            0, inv_y_scale, 0, 0,
            0, 0, 0, (1.0 / draw_distance_near),
            0, 0, -1, 0
         );
      } else {
         return glm::inverse(this->global_state.proj);
      }
   }
   void scene::adjust_camera(const data::camera_coordinate_change& change) {
      this->camera.adjust(change.move, change.turn);
   }

   void scene::update_sun_shadows() {
      glm::vec3 sun_pos  = (this->global_state.sun_dir * -(shadow_draw_distance / 2.0F)) + this->camera.position();
      glm::mat4 sun_view = glm::lookAt(sun_pos, this->camera.position(), glm::vec3(0, 0, 1));
      glm::mat4 sun_proj;
      {
         constexpr float near = 0.01F;
         constexpr float far  = shadow_draw_depth;
         //
         // GLM relies on preprocessor directives to configure things such as whether to 
         // use negative-to-positive  or zero-to-one depth,  and what handedness to use. 
         // However,  those preprocessor directives aren't playing nice,  and I can't be 
         // bothered to figure out why. Instead, I'll just inline the relevant math.
         // 
         // As a bonus, if we were using glm::ortho, we'd have to divide the shadow draw 
         // distance by two, just to (effectively) multiply it back.  No need, this way.
         //
         sun_proj = glm::mat4(1);
         sun_proj[0][0] =  2.0F / shadow_draw_distance;
         sun_proj[1][1] =  2.0F / shadow_draw_distance;
         sun_proj[2][2] = -1.0F / (far - near);
         sun_proj[3][2] = -near / (far - near);
      }
      if constexpr (config::sun_shadow_invert_depth) {
         //
         // The only way to invert an orthographic matrix's depth is to just compute the 
         // matrix normally and then negate the two depth-related terms.
         //
         sun_proj[2][2] = -sun_proj[2][2];
         sun_proj[3][2] = -sun_proj[3][2] + 1.0F;
      }
      //
      this->global_state.sun_space = sun_proj * sun_view;
   }

   void scene::mark_light_shadows_dirty() {
      this->light_shadow_state.set_all_out_of_date();
   }
   void scene::update_light_shadows(frame_in_flight& fif) {
      auto& all_lights = this->entities_of_type<rendered_light>();

      this->light_shadow_state.set_up_to_date(fif.index());
      
      struct _entry {
         size_t index    = index_of_none;
         float  distance = FLT_MAX;
         //
         bool operator>(float d) const noexcept {
            if (index == index_of_none)
               return true;
            return distance > d;
         }
      };
      std::array<_entry, surface_renderer::shadow_caster_count> nearest = { _entry(), _entry(), _entry(), _entry() }; // sorted
      //
      for (size_t i = 0; i < all_lights.size(); ++i) {
         auto& light = all_lights[i];
         if (!light.active())
            continue;
         if (!light.can_cast_shadows())
            continue;
         //
         auto distance = glm::distance((glm::vec3)light.transform()[3], this->camera.position());
         //
         for (size_t j = 0; j < nearest.size(); ++j) {
            auto& entry = nearest[j];
            if (entry > distance) {
               for (size_t k = nearest.size() - 1; k > j; --k) {
                  nearest[k] = nearest[k - 1];
               }
               nearest[j].index    = i;
               nearest[j].distance = distance;
               break;
            }
         }
      }
      //
      // Write shadow view/projection matrices:
      //
      using data_type = std::array<std::array<glm::mat4, 6>, surface_renderer::shadow_caster_count>;
      data_type& data = *(data_type*)fif.shader_params.light_shadow_data.map_memory();
      //
      for (size_t i = 0; i < nearest.size(); ++i) {
         auto& entry = nearest[i];
         if (entry.index == index_of_none) {
            this->global_state.shadow_caster_index[i] = -1;
            data[i] = { glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1) };
            continue;
         }
         assert(entry.index <= all_lights.size());
         auto& light = all_lights[entry.index];
         assert(light.active());
         //
         this->global_state.shadow_caster_index[i] = entry.index;
         //
         constexpr float cubemap_view_fov = glm::radians(90.0F);
         //
         glm::mat4 proj;
         if constexpr (config::light_shadow_invert_depth) {
            constexpr bool infinite_depth = false;

            constexpr float aspect = 1.0F;
            auto y_scale = 1.0F / tan(cubemap_view_fov / 2.0F);
            auto x_scale = y_scale / aspect;
            //
            if constexpr (infinite_depth) {
               proj = glm::mat4(
                  x_scale, 0,       0,  0,
                  0,       y_scale, 0,  0,
                  0,       0,       0, -1,
                  0,       0,       draw_distance_near, 0
               );
            } else {
               float a = draw_distance_near / (light.frame_drawing_data.radius - draw_distance_near);
               float b = (light.frame_drawing_data.radius * draw_distance_near) / (light.frame_drawing_data.radius - draw_distance_near);
               proj = glm::mat4(
                  x_scale, 0,       0,  0,
                  0,       y_scale, 0,  0,
                  0,       0,       a, -1,
                  0,       0,       b,  0
               );
            }
         } else {
            constexpr auto aspect = 1.0F;
            constexpr auto near   = 1.0F;
            //
            // Even though cubemaps are lefthanded, we need a righthanded perspective matrix in order 
            // to get the cubemap faces to face the right directions.
            //
            proj = glm::perspectiveRH_ZO(cubemap_view_fov, aspect, near, light.frame_drawing_data.radius);
         }
         if constexpr (!cubemaps_are_lefthanded) {
            proj[1][1] *= -1;
         }
         //
         const auto inv_position = -glm::vec3(light.transform()[3]);
         for (int j = 0; j < 6; ++j) {
            data[i][j] = proj * glm::translate(common_cubemap_views[j], inv_position);
         }
      }
      fif.shader_params.light_shadow_data.unmap_memory(&data);
   }

   frustum scene::get_current_view_frustum(float near, float far) const {
      frustum out;
      auto&       camera     = this->camera;
      glm::mat4   camera_rot = glm::eulerAngleXYZ(-camera.rotation().x, -camera.rotation().y, -camera.rotation().z);
      const auto& camera_up      = camera_rot[0]; // local Z
      const auto& camera_forward = camera_rot[1]; // local Y
      const auto& camera_right   = camera_rot[2]; // local X
      {
         auto& oa = out.axes;
         oa.up      = camera_up;
         oa.forward = camera_forward;
         oa.right   = camera_right;
      }
      //
      float w_near;
      float h_near;
      float w_far;
      float h_far;
      {
         float two_tan = 2 * tan(this->config.vertical_fov_degrees);
         h_near = two_tan * near;
         w_near = h_near  * this->last_known_view_info.aspect;
         h_far  = two_tan * far;
         w_far  = h_far   * this->last_known_view_info.aspect;
         //
         out.bounds.near = { .w = w_near, .h = h_near };
         out.bounds.far  = { .w = w_far,  .h = h_far };
         //
         {
            float dist = (far - near) / 2.0F + near;
            float h    = two_tan * dist;
            float w    = h * this->last_known_view_info.aspect;
            out.bounds.center = { .w = w, .h = h };
         }
      }
      {
         auto x = camera_right * (w_far / 2.0F);
         auto y = camera_up    * (h_far / 2.0F);
         //
         auto& pf = out.planes.far;
         pf.center = camera.position() + glm::vec3(camera_forward * far);
         pf.upper_left  = pf.center + glm::vec3( y - x);
         pf.upper_right = pf.center + glm::vec3( y + x);
         pf.lower_left  = pf.center + glm::vec3(-y - x);
         pf.lower_right = pf.center + glm::vec3(-y + x);
      }
      {
         auto x = camera_right * (w_near / 2.0F);
         auto y = camera_up    * (h_near / 2.0F);
         //
         auto& pf = out.planes.near;
         pf.center = camera.position() + glm::vec3(camera_forward * near);
         pf.upper_left  = pf.center + glm::vec3( y - x);
         pf.upper_right = pf.center + glm::vec3( y + x);
         pf.lower_left  = pf.center + glm::vec3(-y - x);
         pf.lower_right = pf.center + glm::vec3(-y + x);
      }
      return out;
   }

   void scene::clear(surface_renderer& sr) {
      scene_entities::all_types::for_each([&sr, this]<typename Entity>() {
         auto& list = this->entities_of_type<Entity>();
         if constexpr (std::is_same_v<Entity, rendered_mesh>) {
            auto  size = list.size();
            for (size_t i = 0; i < size; ++i) {
               auto& mesh = list[i];
               if (!mesh.owning_nif)
                  continue;
               mesh.owning_nif->sever_connection_to({ sr, i });
            }
         }
         list.clear();
      });
   }
   void scene::teardown(surface_renderer&sr) {
      //
      // If the scene is being torn down outright, we may not have time to sync with the GPU and 
      // update scene entities manually (e.g. if the program is closing). Just abandon everything.
      //
      scene_entities::all_types::for_each([&sr, this]<typename Entity>() {
         auto& list = this->entities_of_type<Entity>();
         for (auto& item : list) {
            item.lifetime.life_state = scene_entities::life_state::empty;
         }
         list.clear();
      });
   }

   void scene::update() {
      //
      // TODO: Arguably we may want to do this elsewhere; we should eventually separate 
      //       anim_state from the renderer core. You could imagine a tree of NiNode 
      //       structs, some with animation state, which map to a flat list of rendered 
      //       objects in the engine; and you could imagine these structs pushing updates 
      //       elsewhere in the overall flow (or at a fixed tick rate e.g. 60 FPS, etc.).
      //
      auto  now     = std::chrono::high_resolution_clock::now();
      float elapsed = std::chrono::duration<float, std::chrono::seconds::period>(now - this->last_update).count();
      this->last_update = now;
      //
      for (auto& mesh : this->entities_of_type<rendered_mesh>()) {
         if (!mesh.anim_state)
            continue;
         if (!mesh.active())
            continue;
         auto& anim = *mesh.anim_state;
         if (!anim.playing)
            continue;
         anim.elapsed += elapsed;
         if (anim.elapsed > anim.duration)
            anim.elapsed -= anim.duration;
         //
         auto t = mesh.transform();
         t = glm::rotate(t, (elapsed / anim.duration) * glm::radians(360.0f), glm::vec3(0.0f, 0.0f, 1.0f));
         mesh.set_transform(t);
      }
   }

   size_t scene::reuse_scene_texture(const QString& path) {
      auto& list = this->entities_of_type<loaded_texture>();
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& prior = list[i];
         if (prior.path == path) {
            if (prior.active()) {
               return i;
            }
            if (prior.pending_delete()) {
               if (!prior.owned_gpu_resources.current.empty()) {
                  //
                  // A pending-delete entity will have both "current" and "outdated" resources 
                  // if it was marked for delete after recycling began, but before recycling 
                  // could complete. These entitites should be considered irrecoverable and 
                  // allowed to die.
                  //
                  continue;
               }
               if (prior.owned_gpu_resources.outdated.empty()) {
                  #if _DEBUG
                     //
                     // Wait, what? This shouldn't happen. If the entity is pending delete, 
                     // then it wasn't marked as being recycled, so it should either have no 
                     // outdated resources, or have both current and outdated resources, per 
                     // above.
                     //
                     __debugbreak();
                  #endif
                  continue;
               }
               //
               // Rescue the texture from deletion, and reuse it.
               //
               std::swap(
                  prior.owned_gpu_resources.current,
                  prior.owned_gpu_resources.outdated
               );
               prior.lifetime.life_state = scene_entities::life_state::active;
               prior.lifetime.sync_state.set_all_out_of_date();
               --this->entities.pending_deletion_counts.value_for<loaded_texture>();
               //
               return i;
            }
         }
      }
      return index_of_none;
   }

   bool scene::_texture_dec_ref_impl(loaded_texture& tex) {
      assert(tex.refcount != 0 && "About to decrement the refcount into the negatives!");
      if (--tex.refcount == 0) {
         if (tex.persist_for_life_of_renderer())
            return false;
         tex.mark_for_delete();
         ++this->entities.pending_deletion_counts.value_for<loaded_texture>();
         return true;
      }
      return false;
   }
   bool scene::texture_dec_ref(renderer_passkey, loaded_texture& tex) {
      return this->_texture_dec_ref_impl(tex);
   }
   bool scene::texture_dec_ref(loaded_texture_index_passkey, size_t texture_index) {
      auto& tex = this->entities_of_type<loaded_texture>()[texture_index];
      return this->_texture_dec_ref_impl(tex);
   }
}