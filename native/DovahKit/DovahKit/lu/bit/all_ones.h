#pragma once

namespace lu::bits {
   template<typename T, size_t Bitcount>
   constexpr T all_ones() {
      if constexpr (T(1) << Bitcount == 0) {
         return T(-1);
      }
      return (T(1) << Bitcount) - 1;
   }

   template<typename T>
   constexpr T all_ones(size_t bitcount) {
      auto shifted = T(1) << bitcount;
      if (shifted == 0)
         return T(-1);
      return shifted - 1;
   }
}