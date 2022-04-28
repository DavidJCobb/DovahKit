#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#include <glm/glm.hpp>
#include "_vulkan.h"
#include "helpers/array_of_n_values.h" // cobb
#include "helpers/frame_dirty_state.h" // vulkan
#include "helpers/vertex_index_list.h"
#include "helpers/vertex_indices_for_quad_grid.h"
#include "buffer.h"
#include "vertex_landscape.h"
#include "scene_frame_item.h"

namespace dovah::loaded_forms {
   class Landscape;
}

namespace vulkanDK {
   class rendered_landscape {
      public:
         using loaded_form = dovah::loaded_forms::Landscape;
         static constexpr size_t verts_per_side = 33;
         static constexpr size_t verts_per_mesh = verts_per_side * verts_per_side;

         static constexpr size_t indices_per_mesh = vertex_index_count_for_quad_grid<verts_per_side, verts_per_side>;

         static constexpr size_t max_usable_layers_per_quad = 6;

      protected:
         void _on_shader_parameter_change();
      public:
         rendered_landscape() {}

         struct shader_parameters { // pass to the shader via a storage buffer
            alignas(16) glm::vec3 position;
            alignas(4)  uint32_t  pad0C; // padding
            alignas(4)  std::array<int32_t, 4> diffuse_base = { -1, -1, -1, -1 }; // texture indices; one per quad
            alignas(4)  std::array<int32_t, 4> normals_base = { -1, -1, -1, -1 }; // texture indices; one per quad
            union {
               alignas(4) std::array<int32_t, max_usable_layers_per_quad * 4> diffuse_blends = cobb::array_of_n_values<max_usable_layers_per_quad * 4>(-1);
               alignas(4) std::array<std::array<int32_t, max_usable_layers_per_quad>, 4> diffuse_blends_by_quad;
            };
            union {
               alignas(4) std::array<int32_t, max_usable_layers_per_quad * 4> normals_blends = cobb::array_of_n_values<max_usable_layers_per_quad * 4>(-1);
               alignas(4) std::array<std::array<int32_t, max_usable_layers_per_quad>, 4> normals_blends_by_quad;
            };
         };

         std::array<vertex_landscape, verts_per_mesh> vertices;
         shader_parameters shader_params;
         //
         frame_dirty_state handled_frames; // for normal objects: frames that have had shader params synchronized. for pending-delete objects: frames that have been unhooked (when all are unhooked, we can delete the VIB)
         scene_frame_item_state life_state = scene_frame_item_state::empty;

         inline bool active() const noexcept { return this->life_state == scene_frame_item_state::active; }
         inline bool empty() const noexcept { return this->life_state == scene_frame_item_state::empty; }
         inline bool pending_delete() const noexcept { return this->life_state == scene_frame_item_state::pending_delete; }
         
         inline const glm::vec3& position() const noexcept { return this->shader_params.position; }
         void set_position(const glm::vec3&);

         void import_vertex_data_from_form(const loaded_form&);
         void setup_vertex_data_at(void*);

         [[nodiscard]] VkDrawIndexedIndirectCommand make_indirect_draw_command() const;

         void mark_for_delete();
         void reset();

         // World-relative raycasts (uses the mesh's transform):
         bool ray_intersects(const glm::vec3& ray_origin, const glm::vec3& ray_direction, float& hit_distance) const;
   };
}