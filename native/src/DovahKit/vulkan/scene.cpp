#include "scene.h"
//
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
//
#include "config/is_righthanded.h"
#include "config/scene_limits.h"
#include "config/use_inverted_depth.h"
#include "data/DKVulkanCameraUpdate.h"

// for NIF support
#include "nif/file.h"
#include "nif/blocks/NiNode.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/blocks/NiGeometryData.h"

#include <glm/gtx/matrix_decompose.hpp> // for debugging

namespace {
   constexpr float draw_distance_near = 0.1F;
   constexpr float draw_distance_far  = 1000.0F;
}

namespace vulkanDK {
   scene::scene() {
      this->global_state.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
   }

   void scene::update_projection(VkExtent2D render_area) {
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
            // Use infinite far plane:
            //
            auto f = 1.0F / tan(glm::radians(this->config.vertical_fov_degrees) / 2.0F);
            auto t = f / aspect;
            proj = glm::mat4(
               t, 0, 0, 0,
               0, f, 0, 0,
               0, 0, 0, -1,
               0, 0, draw_distance_near, 0
            );
            //
            // sources: <https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/>
            //          <https://dev.theomader.com/depth-precision/>
            //
         } else {
            proj = glm::perspective(glm::radians(this->config.vertical_fov_degrees), aspect, draw_distance_near, draw_distance_far);
         }
         if constexpr (config::is_righthanded) {
            //
            // GLM was designed for OpenGL, which uses an inverted Y axis. We need to flip the 
            // Y-axis here. Do be aware, however, that this is a 3D flip; vertex order will 
            // change handedness (clockwise/counterclockwise), which will affect what Vulkan 
            // considers a "backface" versus a "frontface." You can update the handedness in 
            // the setupGraphicsPipeline function.
            //
            proj[1][1] *= -1;
         }
      }
   }
   void scene::update_camera() {
      auto& cs  = this->camera;
      auto  rot = glm::eulerAngleZYX(-cs.yaw, -cs.roll, -cs.pitch); // negate all three values to turn lefthanded rotations (Skyrim-space) to righthanded (Vulkan-space)
      this->global_state.view = glm::translate(glm::inverse(rot), -cs.position);
   }
   void scene::adjust_camera(const DKVulkanCameraUpdate& change) {
      constexpr float epsilon    = 0.00001;
      constexpr float epsilon_sq = epsilon * epsilon;
      if (change.delta_seconds == 0)
         return;
      //
      auto move    = change.move.direction;
      bool do_move = (glm::length2(move) >= epsilon_sq);
      bool do_turn = change.has_turn();
      //
      if (do_turn) {
         //
         // Continually modifying  a matrix opens us up to floating-point  inaccuracy and 
         // therefore to "creeping roll" within the camera.  Storing bare Euler angles is 
         // a decent enough way to  prevent this, though it means we have to regenerate a 
         // matrix after each camera adjustment.
         //
         const auto& turn = change.turn;
         float speed = turn.speed;
         if (change.turn.scale_by_delta)
            speed *= change.delta_seconds;
         float z = turn.yaw   * speed;
         float y = turn.roll  * speed;
         float x = turn.pitch * speed;
         //
         auto& cs = this->camera;
         cs.yaw   += z;
         cs.roll  += y;
         cs.pitch += x;
         //
         /*//
         constexpr float min_pitch = -85;
         constexpr float max_pitch =  85;
         constexpr float cursed_pitch_extent_a = glm::radians<float>(90 - -min_pitch);
         constexpr float cursed_pitch_extent_b = glm::radians<float>(-(90 + max_pitch));
         cs.pitch = std::clamp(cs.pitch, std::min(cursed_pitch_extent_a, cursed_pitch_extent_b), std::max(cursed_pitch_extent_a, cursed_pitch_extent_b));
         //*/
      }
      if (do_move) {
         //
         // We need to start with a vector that's relative to the camera's reference frame. 
         // However, the camera uses different axes than we expect.
         // 
         // We want to be able to treat the camera as just another object, and objects in 
         // Skyrim use these directions:
         // 
         //  +X = Right
         //  +Y = Forward
         //  +Z = Up
         // 
         // However, the camera's local axes are:
         // 
         //  +X = Right
         //  +Y = Down
         //  +Z = Forward (Depth)
         // 
         // So to start with, we need to swap and possibly negate some axes.
         //
         std::swap(move.z, move.y);
         move.z = -move.z;
         //
         // This turns our "camera-as-object"-relative movement vector into a camera-relative 
         // movement vector.
         //
         float mod = change.move.speed;
         if (change.move.scale_by_delta)
            mod *= change.delta_seconds;
         move = glm::normalize(move) * mod; // NOTE: glm::normalize doesn't check for zero vectors; produces NaN
         //
         // Now, we need to make it world-relative.
         //
         move = glm::inverse(glm::mat3x3(this->global_state.view)) * move;
         this->camera.position += move;
      }
      //
      if (do_move || do_turn) {
         this->update_camera();
      }
   }

   frustrum scene::get_current_view_frustrum(float near, float far) const {
      frustrum out;
      auto& camera     = this->camera;
      auto  camera_rot = glm::eulerAngleXYZ(camera.yaw, camera.roll, camera.pitch);
      const auto& camera_up      = camera_rot[2]; // local Z
      const auto& camera_forward = camera_rot[1]; // local Y
      const auto& camera_right   = camera_rot[0]; // local X
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
      }
      {
         auto x = camera_right * (w_far / 2.0F);
         auto y = camera_up    * (h_far / 2.0F);
         //
         auto& pf = out.planes.far;
         pf.center = camera.position + glm::vec3(camera_forward * far);
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
         pf.center = camera.position + glm::vec3(camera_forward * near);
         pf.upper_left  = pf.center + glm::vec3( y - x);
         pf.upper_right = pf.center + glm::vec3( y + x);
         pf.lower_left  = pf.center + glm::vec3(-y - x);
         pf.lower_right = pf.center + glm::vec3(-y + x);
      }
      return out;
   }

   void scene::teardown() {
      this->lights.clear();
      this->meshes.clear();
      this->textures.clear();
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
      for (auto& mesh : this->meshes) {
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

   size_t scene::insert_new_light() {
      auto& list = this->lights;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.empty())
            return i;
      }
      if (size >= config::max_lights_in_scene)
         return std::string::npos;
      list.emplace_back();
      return size;
   }
   size_t scene::insert_new_mesh() {
      auto& list = this->meshes;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.empty())
            return i;
      }
      if (size >= config::max_rendered_meshes)
         return std::string::npos;
      list.emplace_back();
      return size;
   }
   size_t scene::insert_new_texture() {
      auto& list = this->textures;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i)
         if (list[i].empty())
            return i;
      if (size >= config::max_loaded_textures)
         return std::string::npos;
      list.emplace_back();
      return size;
   }

   size_t scene::_empty_mesh_slot_count() const {
      auto&  list  = this->meshes;
      size_t size  = list.size();
      size_t count = 0;
      for (auto& item : list)
         if (item.empty())
            ++count;
      return count;
   }
   size_t scene::available_mesh_count() const {
      return config::max_rendered_meshes - (this->meshes.size() - this->_empty_mesh_slot_count());
   }
}