#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#include <glm/glm.hpp>
#include "helpers/array_of_n_values.h"
#include "dovah/forms/Landscape.h"
#include "./_vulkan.h"
#include "./helpers/land/vulkan_vertex_indices_for_outline.h"
#include "./helpers/vertex_indices_for_quad_grid.h"
#include "./buffer.h"
#include "./loaded_texture_index.h"
#include "./vertex_landscape.h"

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
         // See below for owned_gpu_resource_coalesce_settings.

      protected:
         static constexpr bool render_as_separated_quads = true;
      public:
         using loaded_form = dovah::loaded_forms::Landscape;

         static constexpr size_t cell_side_length = 4096;
         static constexpr size_t vertex_distance = (cell_side_length / 32);

         // All quads overlap by one line of vertices  on each axis;  this is needed to avoid a gap in tris. 
         // In an ESP file, some vertex data is defined per-cell (e.g. heights) and some is defined per-quad 
         // (e.g. texture blends).  We associate both sets of data with vertices in Vulkan, which means that 
         // vertices at quad boundaries must be separate: two vertices that have the same position, but with 
         // different texturing and blend information.
         static constexpr size_t vertices_per_side = loaded_form::vertices_per_side + 1;
         static constexpr size_t vertices_per_mesh = vertices_per_side * vertices_per_side;

         static constexpr size_t vertices_per_quad_side  = 17;
         static constexpr size_t vertices_per_quad       = vertices_per_quad_side * vertices_per_quad_side;

         static constexpr size_t max_usable_layers_per_quad = 6;

         static constexpr auto quad_vertex_indices = vertex_indices_for_quad_grid<vertices_per_quad_side, vertices_per_quad_side, true>;
         static constexpr auto indices_per_quad    = std::tuple_size_v<decltype(quad_vertex_indices)>;
         using quad_vertex_index_type = decltype(quad_vertex_indices)::value_type;

         static constexpr auto line_vertex_indices = helpers::land::line_vertex_indices;
         static constexpr auto indices_per_line    = std::tuple_size_v<decltype(line_vertex_indices)>;
         static_assert(std::is_same_v<decltype(line_vertex_indices)::value_type, quad_vertex_index_type>);

      #pragma region Scene entity configuration: coalescing
      public:
         static constexpr const auto coalesced_vib_settings = scene_entities::coalesced_vib_settings<1>{
            .enabled = true,
            //
            .additional_shared_index_sets = {
               { indices_per_line },
            },
            .constant_shared_indices = true,
            .fixed_vertex_count      = vertices_per_mesh,
            .fixed_index_count       = indices_per_quad,
         };
         using coalesced_vertex_type = vertex_landscape;
         using coalesced_index_type  = quad_vertex_index_type;

         static void coalesce_constant_shared_indices_into(void* write_to);
         void coalesce_vertices_into(void* write_to) const noexcept;
      #pragma endregion

      protected:
         union blended_texture_list {
            alignas(4) std::array<loaded_texture_index, max_usable_layers_per_quad * 4> all = cobb::array_of_n_values<max_usable_layers_per_quad * 4>(loaded_texture_index{});
            alignas(4) std::array<std::array<loaded_texture_index, max_usable_layers_per_quad>, 4> by_quad;

            blended_texture_list() : all(cobb::array_of_n_values<std::tuple_size_v<decltype(all)>>(loaded_texture_index{})) {}
            ~blended_texture_list() {}
         };

      public:
         rendered_landscape() {}

         void mark_for_delete();
         void reset();

         struct frame_drawing_data_type { // pass to the shader via a storage buffer
            alignas(16) glm::vec3 position;
            alignas(4)  uint32_t  pad0C; // padding
            alignas(4)  std::array<loaded_texture_index, 4> diffuse_base = { loaded_texture_index{}, {}, {}, {} }; // texture indices; one per quad
            alignas(4)  std::array<loaded_texture_index, 4> normals_base = { loaded_texture_index{}, {}, {}, {} }; // texture indices; one per quad
            alignas(4)  blended_texture_list diffuse_blends;
            alignas(4)  blended_texture_list normals_blends;
         };

         std::array<vertex_landscape, vertices_per_mesh> vertices;
         frame_drawing_data_type frame_drawing_data;
         
         inline const glm::vec3& position() const noexcept { return this->frame_drawing_data.position; }
         void set_position(const glm::vec3&);

         void import_vertex_data_from_form(const loaded_form&);

         glm::vec3 local_vertex_position(int quad, size_t quad_vertex_index) const;
         glm::vec3 local_vertex_position(size_t mesh_vertex_index) const;
         glm::vec3 world_vertex_position(int quad, size_t quad_vertex_index) const;
         glm::vec3 world_vertex_position(size_t mesh_vertex_index) const;

         // World-relative raycasts (uses the mesh's transform):
         bool ray_intersects(const glm::vec3& ray_origin, glm::vec3 ray_direction, float& hit_distance) const;
   };
}