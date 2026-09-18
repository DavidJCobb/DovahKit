#pragma once
#include <utility> // std::pair
#include "../../data/landscapes/quad.h"
#include "../../data/landscapes/centerline_vertex_cell_coord.h"
#include "../../data/landscapes/vertices_per_cell_side.h"

namespace dovah::utils::landscapes {
   constexpr bool quad_contains_cell_vertex_coords(dovah::landscapes::quad qi, const std::pair<uint8_t, uint8_t>& cc) {
      using namespace dovah::landscapes;
      const auto& [x, y] = cc;
      if (const bool left = !((size_t)qi & 1)) {
         if (x > centerline_vertex_cell_coord)
            return false;
      } else {
         if (x < centerline_vertex_cell_coord || x >= vertices_per_cell_side)
            return false;
      }
      if (const bool bottom = ((size_t)qi < 2)) {
         if (y > centerline_vertex_cell_coord)
            return false;
      } else {
         if (y < centerline_vertex_cell_coord || y >= vertices_per_cell_side)
            return false;
      }
      return true;
   }
}