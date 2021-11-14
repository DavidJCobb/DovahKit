#include "scene.h"
//
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
//
#include "config/scene_limits.h"
#include "data/DKVulkanCameraUpdate.h"

#include <glm/gtx/matrix_decompose.hpp> // for debugging

namespace vulkanDK {
   scene::scene() {
      this->global_state.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
   }

   void scene::update_projection(VkExtent2D render_area) {
      float aspect = 1.0F;
      if (render_area.height != 0.0)
         aspect = (float)render_area.width / (float)render_area.height;
      this->global_state.proj = glm::perspective(glm::radians(this->config.vertical_fov_degrees), aspect, 0.1f, 10.0f);
      //
      // GLM was designed for OpenGL, which uses an inverted Y axis. We need to flip the 
      // Y-axis here. Do be aware, however, that this is a 3D flip; vertex order will 
      // change handedness (clockwise/counterclockwise), which will affect what Vulkan 
      // considers a "backface" versus a "frontface." You can update the handedness in 
      // the setupGraphicsPipeline function.
      //
      this->global_state.proj[1][1] *= -1;
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
         if (mesh.pending_delete || mesh.empty())
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
   void scene::adjust_camera(const DKVulkanCameraUpdate& change) {
      constexpr float epsilon    = 0.00001;
      constexpr float epsilon_sq = epsilon * epsilon;
      //
      auto& camera = this->global_state.view;
      //
      bool do_move = true;
      bool do_turn = true;
      auto move = change.move.direction;
      auto turn = change.turn.rotation;
      if (change.delta_seconds) {
         do_move = (glm::length2(move) >= epsilon_sq);
         do_turn = (glm::length2(turn) >= epsilon_sq);
         if (do_turn) {
            turn = glm::normalize(turn) * (change.turn.speed * change.delta_seconds);
         }
      }
      //
      if (do_turn) {
         auto rot = glm::eulerAngleZY(turn.z, turn.y);
         rot *= glm::eulerAngleX(turn.x);
         //
         camera = rot * camera;
      }
      if (do_move) {
         move = glm::inverse(glm::mat3x3(camera)) * move;
         if (change.delta_seconds) {
            move = glm::normalize(move) * (change.move.speed * change.delta_seconds); // NOTE: glm::normalize doesn't check for zero vectors; produces NaN
         }
         camera = glm::translate(camera, move);
      }
   }

   size_t scene::insert_new_mesh() {
      auto& list = this->meshes;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.empty() && !item.pending_delete)
            return i;
      }
      if (size >= config::max_rendered_meshes)
         return std::string::npos;
      list.emplace_back();
      {
         list.back().anim_state = new mesh_animation_state; // TODO: this is just for testing purposes
      }
      return size;
   }
   size_t scene::insert_new_texture() {
      auto& list = this->textures;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.content.handle == VK_NULL_HANDLE && !item.pending_delete)
            return i;
      }
      if (size >= config::max_loaded_textures)
         return std::string::npos;
      list.emplace_back();
      return size;
   }
}