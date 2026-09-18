#pragma once
#include <utility> // std::pair
#include "../../data/landscapes/quad.h"
#include "../../data/landscapes/vertices_per_quad_side.h"

namespace dovah::utils::landscapes {
   // Biases toward top-right for vertices on the centerlines.
   constexpr dovah::landscapes::quad containing_quad_of_cell_vertex_coords(const std::pair<uint8_t, uint8_t>& cc) {
      using namespace dovah::landscapes;
      const auto& [x, y] = cc;
      uint8_t v = 0;
      if (x >= vertices_per_quad_side)
         v |= 1;
      if (y >= vertices_per_quad_side)
         v |= 2;
      return (dovah::landscapes::quad)v;
   }
}