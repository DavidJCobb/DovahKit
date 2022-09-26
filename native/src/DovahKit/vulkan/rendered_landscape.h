#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#include <glm/glm.hpp>
#include "_vulkan.h"
#include "helpers/array_of_n_values.h" // cobb
#include "helpers/vertex_indices_for_quad_grid.h"
#include "buffer.h"
#include "vertex_landscape.h"
#include "scene_frame_item.h"

#include "./scene_entities/base.h"

namespace dovah::loaded_forms {
   class Landscape;
}

namespace vulkanDK {
   class rendered_landscape : public scene_entities::base {
      public:
         static constexpr const char* name_single = "landscape";
         static constexpr const char* name_plural = "landscapes";
         //
         static constexpr const bool owned_gpu_resources_are_coalesced = true;
         static constexpr const bool is_drawn = true;
      protected:
         static constexpr bool render_as_separated_quads = true;
      public:
         using loaded_form = dovah::loaded_forms::Landscape;
         static constexpr size_t vertices_per_side = 33 + 1; // all quads overlap by one line of vertices on each axis; needed to avoid a gap in tris
         static constexpr size_t vertices_per_mesh = vertices_per_side * vertices_per_side;

         static constexpr size_t vertices_per_quad_side  = 17;
         static constexpr size_t vertices_per_quad       = vertices_per_quad_side * vertices_per_quad_side;

         static constexpr size_t max_usable_layers_per_quad = 6;

         static constexpr auto quad_vertex_indices = vertex_indices_for_quad_grid<vertices_per_quad_side, vertices_per_quad_side, true>;
         static constexpr auto indices_per_quad    = std::tuple_size_v<decltype(quad_vertex_indices)>;
         using quad_vertex_index_type = decltype(quad_vertex_indices)::value_type;

         static constexpr size_t cell_side_length = 4096;
         static constexpr size_t vertex_distance  = (cell_side_length / 32);

      protected:
         void _on_shader_parameter_change();
      public:
         rendered_landscape() {}

         void mark_for_delete();
         void reset();

         struct frame_drawing_data_type { // pass to the shader via a storage buffer
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

         std::array<vertex_landscape, vertices_per_mesh> vertices;
         frame_drawing_data_type frame_drawing_data;
         
         inline const glm::vec3& position() const noexcept { return this->frame_drawing_data.position; }
         void set_position(const glm::vec3&);

         void import_vertex_data_from_form(const loaded_form&);
         void setup_vertex_data_at(void*);

         [[nodiscard]] VkDrawIndexedIndirectCommand make_indirect_draw_command() const;

         glm::vec3 local_vertex_position(int quad, size_t quad_vertex_index) const;
         glm::vec3 local_vertex_position(size_t mesh_vertex_index) const;
         glm::vec3 world_vertex_position(int quad, size_t quad_vertex_index) const;
         glm::vec3 world_vertex_position(size_t mesh_vertex_index) const;

         // World-relative raycasts (uses the mesh's transform):
         bool ray_intersects(const glm::vec3& ray_origin, glm::vec3 ray_direction, float& hit_distance) const;
   };
}