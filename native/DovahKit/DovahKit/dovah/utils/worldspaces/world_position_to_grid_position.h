#pragma once
#include <cstdint>
#include <utility>
#include "./world_coordinate_to_grid_coordinate.h"
#include "dovah/types/cells/grid_position.h"

namespace dovah::utils::worldspaces {
   constexpr types::cells::grid_position world_position_to_grid_position(float x, float y) noexcept {
      return { world_coordinate_to_grid_coordinate(x), world_coordinate_to_grid_coordinate(y) };
   }
}