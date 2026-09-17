#pragma once
#include "dovah/data/landscapes/vertices_per_cell_side.h"
#include "dovah/data/landscapes/vertices_per_quad.h"
#include "dovah/data/landscapes/vertices_per_quad_side.h"

namespace vulkanDK::helpers::land {

   //
   // Vertex indices are not 1:1 between a loaded Landscape form and a `rendered_landscape` 
   // entity in our renderer. The reason for this has to do with how certain landscape data 
   // is associated with vertices.
   // 
   // Conceptually, a landscape is a 33x33-vertex grid, where two edges are shared with 
   // neighboring landscapes (to avoid gaps between the meshes). However, landscapes are 
   // divided into four "quads," each of which has a 17x17-vertex mesh; these meshes share 
   // vertices at the landscape's centerlines.
   // 
   // Within game data, vertex heights, normals, and colors are all defined with respect 
   // to a 33x33 grid. However, textures and texture blend opacities are defined per-quad, 
   // though they still use vertex indices within a 33x33 grid.
   // 
   // Within our renderer, texture indices and texture blend opacities are passed as vertex 
   // attributes. This means that the vertices that are shared between quads need to be 
   // "split apart" in order for us to properly render a landscape's textures at quad edges. 
   // That is: height, normal, and color values must be duplicated, and then different blend 
   // information must be associated with each duplicate. The result is four 17x17-vertex 
   // meshes: a single vertex list is used, but vertices are stored one quad after another.
   // 
   // The helper functions below, then, exist to convert between the two different vertex 
   // indexing schemes: per-landscape (i.e. how vertices are indexed in the game data) and 
   // Vulkan-side (i.e. how vertices are indexed within a `rendered_landscape` entity's 
   // vertex list).



   // Index of the vertex row/column (within the 33x33 grid) that is shared between adjacent 
   // quads.
   inline constexpr size_t centerline_rowcol_index = (dovah::landscapes::vertices_per_cell_side - 1) / 2;

   // Parameters:
   // 
   //  - A quad index [0, 3].
   //  - A quad-relative vertex column number (X-coordinate / distance between vertices).
   //  - A quad-relative vertex row    number (Y-coordinate / distance between vertices).
   // 
   // Results:
   // 
   //  - A per-landscape vertex index `land_vi`. Given a landscape form `form`, you could 
   //    get the vertex's height as `form.heightmap.heights.by_flat_index(land_vi)`.
   // 
   //  - A Vulkan-side vertex index `vulkan_vi`. This is the index of the corresponding 
   //    vertex within a `rendered_landscape` entity's local vertex array.
   //
   constexpr void map_vertex_coords_to_vertex_indices(size_t quad, size_t quad_x, size_t quad_y, size_t& land_vi, size_t& vulkan_vi) {
      size_t offset_x = (quad % 2) * centerline_rowcol_index;
      size_t offset_y = (quad / 2) * centerline_rowcol_index;

      land_vi   = ((quad_y + offset_y) * dovah::landscapes::vertices_per_cell_side) + (quad_x + offset_x);
      vulkan_vi = (quad * dovah::landscapes::vertices_per_quad) + (quad_y * dovah::landscapes::vertices_per_quad_side) + quad_x;
   }

   // Given a quad index [0, 3] and a per-cell vertex index, this function returns the 
   // Vulkan-side vertex index for that vertex in that quad.
   //
   constexpr size_t vulkan_vertex_index_for_per_land_vertex_index(size_t quad, size_t per_land_index) {
      constexpr size_t centerline_rowcol_index = (dovah::landscapes::vertices_per_cell_side - 1) / 2;

      size_t offset_x = (quad % 2) * centerline_rowcol_index;
      size_t offset_y = (quad / 2) * centerline_rowcol_index;

      size_t y = per_land_index / dovah::landscapes::vertices_per_cell_side - offset_y;
      size_t x = per_land_index % dovah::landscapes::vertices_per_cell_side - offset_x;

      return (quad * dovah::landscapes::vertices_per_quad) + (y * dovah::landscapes::vertices_per_quad_side) + x;
   }

   //
   // Correctness checks for mapping vertex coordinates to vertex indices, and for 
   // mapping Vulkan vertex indices to per-land vertex indices.
   //
   static_assert(
      []() -> bool {
         for (size_t q = 0; q < 4; ++q) {
            for (size_t y = 0; y < dovah::landscapes::vertices_per_quad_side; ++y) {
               for (size_t x = 0; x < dovah::landscapes::vertices_per_quad_side; ++x) {
                  size_t land_vi   = 0;
                  size_t vulkan_vi = 0;
                  map_vertex_coords_to_vertex_indices(q, x, y, land_vi, vulkan_vi);

                  if (vulkan_vertex_index_for_per_land_vertex_index(q, land_vi) != vulkan_vi)
                     return false;
               }
            }
         }
         return true;
      }()
   );
}
