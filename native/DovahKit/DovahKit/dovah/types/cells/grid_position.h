#pragma once
#include <cstdint>

namespace dovah::types::cells {
   struct grid_position {
      int32_t x = 0;
      int32_t y = 0;

      // A 32-bit value used to group cells by their grid positions within data files.
      constexpr uint32_t to_cell_block() const noexcept {
         int16_t xs = x / 8 / 4;
         int16_t ys = y / 8 / 4;
         return ((uint32_t)xs << 16) | ys;
      }

      // A 32-bit value used to group cells by their grid positions within data files.
      constexpr uint32_t to_cell_sub_block() const noexcept {
         int16_t xs = x / 8;
         int16_t ys = y / 8;
         return ((uint32_t)xs << 16) | ys;
      }
   };
}
