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
      {
         auto& proj = this->global_state.proj;
         proj = glm::perspective(glm::radians(this->config.vertical_fov_degrees), aspect, draw_distance_near, draw_distance_far);
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
         }
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
   void scene::update_camera() {
      auto& cs = this->camera;
      //
      // Apply a lefthanded extrinsic ZYX Euler rotation:
      //
      auto rot = glm::eulerAngleZ(cs.yaw);   // extrinsic ZY(X) = intrinsic XY(Z)
      rot     *= glm::eulerAngleY(cs.roll);  // extrinsic Z(Y)X = intrinsic X(Y)Z
      rot     *= glm::eulerAngleX(cs.pitch); // extrinsic (Z)YX = intrinsic (X)YZ
      //
      // Create the final view matrix.
      // 
      //this->global_state.view = glm::inverse(glm::translate(rot, cs.position));
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
         cs.yaw   -= z; // assume lefthanded (clockwise) input. TODO: do we have to transpose the matrix in (update_camera) to switch handedness?
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

   void scene::teardown() {
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