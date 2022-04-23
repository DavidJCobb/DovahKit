#pragma once
#include <chrono>
#include <vector>
#include "buffer.h"
#include "frustum.h"
#include "loaded_texture.h"
#include "rendered_bounds.h"
#include "rendered_light.h"
#include "rendered_mesh.h"
#include "scene_global_state.h"

struct DKVulkanCameraUpdate;
namespace nifDK {
   class file;
}

namespace vulkanDK {
   class frame_in_flight;
   class surface_renderer;

   class scene {
      public:
         scene();

         std::chrono::steady_clock::time_point last_update;
         std::vector<rendered_bounds> bounds; // bounding boxes, to indicate object selections
         std::vector<rendered_light>  lights;
         std::vector<rendered_mesh>   meshes;
         std::vector<loaded_texture>  textures;
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
         //
         struct {
            size_t bounds   = 0; // count
            size_t lights   = 0; // count
            size_t meshes   = 0; // count
            size_t textures = 0; // count
         } pending_deletions;
         frame_dirty_state light_shadow_state;

         void update_projection(VkExtent2D render_area);
         void update_camera();
         void adjust_camera(const DKVulkanCameraUpdate&);

         void update_sun_shadows();

         void mark_light_shadows_dirty();
         void update_light_shadows(frame_in_flight&);

         frustum get_current_view_frustum(float near, float far) const;

         void teardown();
         void update(); // anim state, etc.

         size_t insert_new_bound();   // returns index of inserted item, std::string::npos on failure
         size_t insert_new_light();   // returns index of inserted item, std::string::npos on failure
         size_t insert_new_mesh();    // returns index of inserted item, std::string::npos on failure
         size_t insert_new_texture(); // returns index of inserted item, std::string::npos on failure

      protected:
         size_t _empty_mesh_slot_count() const;
      public:
         size_t available_mesh_count() const;
   };
}