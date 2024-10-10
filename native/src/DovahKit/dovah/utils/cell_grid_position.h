#pragma once
#include <cstdint>

namespace dovah {
   struct cell_grid_position {
      int32_t x = 0;
      int32_t y = 0;

      // A 32-bit value used to group cells by their grid positions within data files.
      constexpr uint32_t to_cell_block() const noexcept;

      // A 32-bit value used to group cells by their grid positions within data files.
      constexpr uint32_t to_cell_sub_block() const noexcept;
   };
}

#include "./cell_grid_position.inl"
