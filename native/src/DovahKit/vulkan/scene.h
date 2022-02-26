#pragma once
#include <chrono>
#include <vector>
#include "frustrum.h"
#include "loaded_texture.h"
#include "rendered_light.h"
#include "rendered_mesh.h"
#include "scene_global_state.h"
#include "scene_shadow_state.h"

struct DKVulkanCameraUpdate;
namespace nifDK {
   class file;
}

namespace vulkanDK {
   class scene {
      public:
         scene();

         std::chrono::steady_clock::time_point last_update;
         std::vector<rendered_light> lights;
         std::vector<rendered_mesh>  meshes;
         std::vector<loaded_texture> textures;
         //
         struct {
            float vertical_fov_degrees = 45.0F;
         } config;
         struct {
            VkExtent2D bounds = {};
            float      aspect = 0;
         } last_known_view_info;
         struct {
            float yaw   = 0.0; // clockwise; heading
            float pitch = 0.0; // clockwise; nose up/down
            float roll  = 0.0; // clockwise; sideways lean
            glm::vec3 position = { 0, 0, 0 };
         } camera;
         scene_global_state global_state; // GPU-side state
         scene_shadow_state shadow_state; // GPU-side state for directional sun's shadows
         //
         struct {
            size_t lights   = 0; // count
            size_t meshes   = 0; // count
            size_t textures = 0; // count
         } pending_deletions;

         void update_projection(VkExtent2D render_area);
         void update_camera();
         void adjust_camera(const DKVulkanCameraUpdate&);

         void update_sun_shadows();

         frustrum get_current_view_frustrum(float near, float far) const;

         void teardown();
         void update(); // anim state, etc.

         size_t insert_new_light();   // returns index of inserted item, std::string::npos on failure
         size_t insert_new_mesh();    // returns index of inserted item, std::string::npos on failure
         size_t insert_new_texture(); // returns index of inserted item, std::string::npos on failure

      protected:
         size_t _empty_mesh_slot_count() const;
      public:
         size_t available_mesh_count() const;
   };
}