#pragma once
#include "./cell_grid_position.h"

namespace dovah {
   constexpr uint32_t cell_grid_position::to_cell_block() const noexcept {
      int16_t xs = x / 8 / 4;
      int16_t ys = y / 8 / 4;
      return ((uint32_t)xs << 16) | ys;
   }
   constexpr uint32_t cell_grid_position::to_cell_sub_block() const noexcept {
      int16_t xs = x / 8;
      int16_t ys = y / 8;
      return ((uint32_t)xs << 16) | ys;
   }
}