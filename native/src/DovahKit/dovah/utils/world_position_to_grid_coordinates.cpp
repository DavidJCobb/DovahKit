#include "world_position_to_grid_coordinates.h"
#include "../forms/Cell.h"

namespace dovah {
   namespace {
      static constexpr int32_t side_length = loaded_forms::Cell::side_length;

      int32_t _coordinate(float v) {
          if constexpr (side_length == 4096) {
            //
            // If the cell size is 4096, then we can just right-shift by 0xC (sign permitting).
            //
            if constexpr (int32_t(-90) >> 2 == -23) {
               //
               // Right-shifting a negative number fills  the shifted-in bits with the sign bit 
               // on this compiler and platform. Bethesda shifts right by 0xC, so we can do the 
               // same.
               //
               return (int32_t)v >> 0xC;
            } else {
               if (v >= 0)
                  return (int32_t)v >> 0xC;
               return ((int32_t)v / side_length) - 1;
            }
         }
         auto out = (int32_t)v / side_length;
         if (v < 0)
            --out;
         return out;
      }
   }

   extern std::pair<int32_t, int32_t> world_position_to_grid_coordinates(float x, float y) {
      return { _coordinate(x), _coordinate(y) };
   }
}