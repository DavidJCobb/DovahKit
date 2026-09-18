#pragma once
#include <utility> // std::pair
#include "../../data/landscapes/quad.h"
#include "../../data/landscapes/centerline_vertex_cell_coord.h"

namespace dovah::utils::landscapes {
   constexpr std::pair<uint8_t, uint8_t> cell_vertex_coords_to_quad_vertex_coords(dovah::landscapes::quad qi, std::pair<uint8_t, uint8_t> cc) {
      using namespace dovah::landscapes;
      auto& [x, y] = cc;
      switch (qi) {
         case quad::bottom_left:
            break;
         case quad::top_left:
            y -= centerline_vertex_cell_coord;
            break;
         case quad::bottom_right:
            x -= centerline_vertex_cell_coord;
            break;
         case quad::top_right:
            x -= centerline_vertex_cell_coord;
            y -= centerline_vertex_cell_coord;
            break;
      }
      return { x, y };
   }
}