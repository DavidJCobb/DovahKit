#pragma once
#include <chrono>
#include <limits>
#include <vector>
#include "helpers/tuples/map_types.h"
#include "helpers/class_map.h"
#include "helpers/passkey.h"
#include "./config/scene_limits.h"
#include "./scene_entities/all_types.h"
#include "./scene_entities/fif_sync_state.h"
#include "./frustum.h"
#include "./loaded_texture.h"
#include "./loaded_texture_index.h"
#include "./rendered_bounds.h"
#include "./rendered_landscape.h"
#include "./rendered_light.h"
#include "./rendered_mesh.h"
#include "./scene_global_state.h"

struct DKVulkanCameraUpdate;
namespace nifDK {
   class file;
}

namespace vulkanDK {
   class frame_in_flight;
   class surface_renderer;

   namespace impl::_scene {
      template<typename T> struct to_vector {
         using type = std::vector<T>;
      };
   }

   class scene {
      public:
         static constexpr size_t index_of_none = std::numeric_limits<size_t>::max();

         using loaded_texture_index_passkey = cobb::passkey<loaded_texture_index, scene>;
         using renderer_passkey = cobb::passkey<surface_renderer, scene>;

      public:
         scene();

         std::chrono::steady_clock::time_point last_update = {};
         struct {
            cobb::tuples::map_types<impl::_scene::to_vector, scene_entities::all_types::as_tuple> lists;
            cobb::class_map_from_class_array<size_t, scene_entities::all_types> pending_deletion_counts;
         } entities;
         //
         struct {
            float  vertical_fov_degrees      = 45.0F; // call scene::update_projection after setting
            int8_t landscape_grid_side_count = config::initial_landscape_side_count; // must set via surface_renderer::set_landscape_grid_side_count
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
         scene_entities::fif_sync_state light_shadow_state;

         template<typename Entity> std::vector<Entity>& entities_of_type() {
            return std::get<std::vector<Entity>>(this->entities.lists);
         }
         template<typename Entity> const std::vector<Entity>& entities_of_type() const {
            return std::get<std::vector<Entity>>(this->entities.lists);
         }

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

         // Attempts to insert a new scene entity of the given type. If successful, returns the 
         // index of the inserted item, which will have its life state set to active (or active 
         // and recycling). If unsuccessful, returns `index_of_none`.
         template<typename Entity> size_t insert_new_scene_entity();

         // If the specified texture is loaded, its index will be returned. If the texture is 
         // pending deletion, it will be rescued from deletion.
         size_t reuse_scene_texture(const QString& path);

         template<typename Entity> size_t max_entity_slots() const noexcept;
         template<typename Entity> size_t entity_slots_available() const noexcept;

      protected:
         bool _texture_dec_ref_impl(loaded_texture&);
      public:
         bool texture_dec_ref(renderer_passkey, loaded_texture&);
         bool texture_dec_ref(loaded_texture_index_passkey, size_t texture_index); // returns true if the texture will be marked for delete
   };
}

#include "scene.inl"