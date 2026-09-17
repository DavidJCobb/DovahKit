#pragma once
#include <utility> // std::pair
#include "../../data/landscapes/quad.h"
#include "../../data/landscapes/vertices_per_quad_side.h"

namespace dovah::utils::landscapes {
   constexpr std::pair<uint8_t, uint8_t> cell_vertex_coords_to_quad_vertex_coords(dovah::landscapes::quad qi, std::pair<uint8_t, uint8_t> cc) {
      using namespace dovah::landscapes;
      auto& [x, y] = cc;
      switch (qi) {
         case quad::bottom_left:
            break;
         case quad::top_left:
            y -= vertices_per_quad_side - 1;
            break;
         case quad::bottom_right:
            x -= vertices_per_quad_side - 1;
            break;
         case quad::top_right:
            x -= vertices_per_quad_side - 1;
            y -= vertices_per_quad_side - 1;
            break;
      }
      return { x, y };
   }
}