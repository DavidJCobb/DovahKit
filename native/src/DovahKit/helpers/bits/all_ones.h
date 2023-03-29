#pragma once

namespace cobb::bits {
   template<size_t Bitcount, typename T> constexpr T all_ones() {
      if constexpr (T(1) << Bitcount == 0) {
         return T(-1);
      }
      return (T(1) << Bitcount) - 1;
   }

   template<typename T, bool CheckOverflow = false> constexpr T all_ones(size_t bitcount) {
      auto shifted = T(1) << bitcount;
      if constexpr (CheckOverflow) {
         if (shifted == 0)
            return T(-1);
      }
      return shifted - 1;
   }
}