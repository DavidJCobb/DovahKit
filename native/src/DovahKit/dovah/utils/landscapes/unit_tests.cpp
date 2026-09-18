#include "./cell_vertex_coords_to_quad_vertex_coords.h"
#include "./containing_quad_of_cell_vertex_coords.h"
#include "./quad_vertex_coords_to_cell_vertex_coords.h"
#include "../../data/landscapes/centerline_vertex_cell_coord.h"
#include "../../data/landscapes/vertices_per_cell_side.h"
#include "../../data/landscapes/vertices_per_quad_side.h"

namespace {
   using namespace dovah::landscapes;
   using namespace dovah::utils::landscapes;

   static constexpr bool vertex_round_trips(quad q, size_t cx, size_t cy) {
      auto qc = cell_vertex_coords_to_quad_vertex_coords(q, { cx, cy });
      auto cc = quad_vertex_coords_to_cell_vertex_coords(q, qc);
      if (cc.first != cx)
         return false;
      if (cc.second != cy)
         return false;
      return true;
   }

   static_assert(
      []() -> bool {
         for (size_t x = 0; x < vertices_per_cell_side; ++x) {
            for (size_t y = 0; y < vertices_per_cell_side; ++y) {
               auto q  = containing_quad_of_cell_vertex_coords({ x, y });
               if (!vertex_round_trips(q, x, y))
                  return false;
            }
         }
         return true;
      }(),
      "Conversions between cell and quad vertex coords should round-trip properly."
   );

   // Centerlines need extra checks because they exist in two or more quads
   static_assert(
      []() -> bool {
         const size_t x = centerline_vertex_cell_coord;
         for (size_t y = 0; y < vertices_per_cell_side; ++y) {
            if (y <= centerline_vertex_cell_coord) {
               if (!vertex_round_trips(quad::bottom_left, x, y))
                  return false;
               if (!vertex_round_trips(quad::bottom_right, x, y))
                  return false;
            }
            if (y >= centerline_vertex_cell_coord) {
               if (!vertex_round_trips(quad::top_left, x, y))
                  return false;
               if (!vertex_round_trips(quad::top_right, x, y))
                  return false;
            }
         }
         return true;
      }(),
      "Conversions between cell and quad vertex coords should round-trip properly. (Centerline case.)"
   );
}