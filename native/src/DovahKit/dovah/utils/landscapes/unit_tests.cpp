#include "./cell_vertex_coords_to_quad_vertex_coords.h"
#include "./containing_quad_of_cell_vertex_coords.h"
#include "./quad_vertex_coords_to_cell_vertex_coords.h"
#include "../../data/landscapes/vertices_per_cell_side.h"
#include "../../data/landscapes/vertices_per_quad_side.h"

namespace {
   using namespace dovah::landscapes;
   using namespace dovah::utils::landscapes;

   static_assert(
      []() -> bool {
         for (size_t x = 0; x < vertices_per_cell_side; ++x) {
            for (size_t y = 0; y < vertices_per_cell_side; ++y) {
               auto q  = containing_quad_of_cell_vertex_coords({ x, y });
               auto qc = cell_vertex_coords_to_quad_vertex_coords(q, { x, y });
               auto cc = quad_vertex_coords_to_cell_vertex_coords(q, qc);
               if (cc.first != x)
                  return false;
               if (cc.second != y)
                  return false;
            }
         }
         return true;
      }(),
      "Conversions between cell and quad vertex coords should round-trip properly."
   );
}