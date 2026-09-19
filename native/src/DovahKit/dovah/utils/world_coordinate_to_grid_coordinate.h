#pragma once
#include <cstdint>
#include "../core_constants/exterior_cell_side_length.h"

namespace dovah {

   // Given a world coordinate (X or Y), produces the containing grid coordinate on the same axis. 
   // Don't try to do this yourself; the "obvious" approach doesn't work.
   constexpr int32_t world_coordinate_to_grid_coordinate(float c) {
      //
      // This isn't as simple as (int)(c / side_length), because that will fail to handle 
      // negative numbers, i.e. given world coordinates (-64, -64), that produces the wrong 
      // grid coordinates (0, 0) instead of (-1, -1).
      //
      constexpr auto side_length = core_constants::exterior_cell_side_length;
      //
      constexpr auto shift_by     = std::bit_width((unsigned int)side_length) - 1;
      constexpr bool signed_shift = ((int32_t)(-90) >> 2 == -23);
      //
      auto ci = (int32_t)c;
      if constexpr (side_length == (1 << shift_by)) { // optimized implementation
         if constexpr (!signed_shift) {
            //
            // Not all compilers extend the sign bit when shifting to the left. If we're 
            // on a compiler that doesn't, then we must handle negative values manually.
            //
            if (c < 0)
               return (ci / side_length) - 1;
         }
         auto out = ci >> shift_by;
         if (ci < 0 && ci > -side_length)
            --out;
         return out;
      } else { // "canonical" implementation
         auto out = ci / side_length;
         if (ci < 0 && ci > -side_length)
            --out;
         return out;
      }
   }

}