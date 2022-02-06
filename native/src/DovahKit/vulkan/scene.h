#pragma once
#include <chrono>
#include <vector>
#include "loaded_texture.h"
#include "rendered_mesh.h"
#include "scene_global_state.h"

struct DKVulkanCameraUpdate;
namespace nifDK {
   class file;
}

namespace vulkanDK {
   class scene {
      public:
         scene();

         std::chrono::steady_clock::time_point last_update;
         std::vector<rendered_mesh>  meshes;
         std::vector<loaded_texture> textures;
         //
         struct {
            float vertical_fov_degrees = 45.0F;
         } config;
         struct {
            float yaw   = 0.0; // heading
            float pitch = 0.0; // nose up/down
            float roll  = 0.0; // sideways lean
            glm::vec3 position = { 0, 0, 0 };
         } camera;
         scene_global_state global_state; // GPU-side state
         //
         struct {
            size_t meshes   = 0; // count
            size_t textures = 0; // count
         } pending_deletions;

         void update_projection(VkExtent2D render_area);
         void update_camera();
         void adjust_camera(const DKVulkanCameraUpdate&);

         void teardown();
         void update(); // anim state, etc.

         size_t insert_new_mesh(); // returns std::string::npos on failure
         size_t insert_new_texture(); // returns std::string::npos on failure

      protected:
         size_t _empty_mesh_slot_count() const;
      public:
         size_t available_mesh_count() const;
   };
}