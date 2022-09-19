#pragma once
#include <chrono>
#include <limits>
#include <vector>
#include "helpers/passkey.h"
#include "buffer.h"
#include "frustum.h"
#include "loaded_texture.h"
#include "rendered_bounds.h"
#include "rendered_landscape.h"
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
         static constexpr size_t index_of_none = std::numeric_limits<size_t>::max();

         using renderer_passkey = cobb::passkey<surface_renderer, scene>;

      public:
         scene();

         std::chrono::steady_clock::time_point last_update = {};
         std::vector<rendered_bounds>    bounds;     // bounding boxes, to indicate object selections
         std::vector<rendered_landscape> landscapes; // heightmapped terrain generated from LAND forms
         std::vector<rendered_light>     lights;     // in-scene light emitters, including ones that support shadow casting
         std::vector<rendered_mesh>      meshes;
         std::vector<loaded_texture>     textures;   // all textures used within the scene, aside from things like shadow maps that we generate and update during the render
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
            buffer landscape_buffer; // indices; then all verts
         } coalesced;
         struct {
            size_t bounds     = 0; // count
            size_t landscapes = 0; // count
            size_t lights     = 0; // count
            size_t meshes     = 0; // count
            size_t textures   = 0; // count
         } pending_deletions;
         frame_dirty_state light_shadow_state;

         void setup_landscape_buffer(surface_renderer&, size_t max_landscape_count);

         void update_projection(VkExtent2D render_area);
         void update_camera();
         void adjust_camera(const DKVulkanCameraUpdate&);

         void update_sun_shadows();

         void mark_light_shadows_dirty();
         void update_light_shadows(frame_in_flight&);

         frustum get_current_view_frustum(float near, float far) const;

         void clear(surface_renderer&);
         void teardown(surface_renderer&);
         void update(); // anim state, etc.

         size_t insert_new_bound();     // returns index of inserted item; index_of_none on failure
         size_t insert_new_landscape(); // returns index of inserted item; index_of_none on failure
         size_t insert_new_light();     // returns index of inserted item; index_of_none on failure
         size_t insert_new_mesh();      // returns index of inserted item; index_of_none on failure
         size_t insert_new_texture();   // returns index of inserted item; index_of_none on failure

         bool texture_dec_ref(renderer_passkey, loaded_texture&); // returns true if the texture will be marked for delete

         size_t landscape_buffer_vertex_index(size_t landscape_index) const;
         void update_single_landscape(surface_renderer&, size_t landscape_index);

      protected:
         size_t _empty_mesh_slot_count() const;
      public:
         size_t available_mesh_count() const;
   };
}