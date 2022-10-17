#pragma once
#include <cstdint>
#include <utility>
#include "./world_coordinate_to_grid_coordinate.h"

namespace dovah {
   constexpr std::pair<int32_t, int32_t> world_position_to_grid_coordinates(float x, float y) {
      return { world_coordinate_to_grid_coordinate(x), world_coordinate_to_grid_coordinate(y) };
   }
}