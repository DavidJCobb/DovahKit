#pragma once
#include <cstdint>
#include <utility>

namespace dovah {
   extern std::pair<int32_t, int32_t> world_position_to_grid_coordinates(float x, float y);
}