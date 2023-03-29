#pragma once
#include "./all_ones.h"

namespace cobb::bits {
   // Replaces the most significant bits (i.e. leftmost) in `dst` with the bits in `src`. 
   // The `bitcount` argument is the number of bits to replace.
   template<typename T, size_t DstBitcount = (sizeof(T) * 8)>
   constexpr T replace_most_significant_bits(T dst, T src, size_t bitcount) {
      auto mask  = all_ones<T>(bitcount);
      auto shift = DstBitcount - bitcount;

      dst &= ~(mask << shift);
      dst |= (src & mask) << shift;
      return dst;
   }

   // Replaces the most significant bits (i.e. leftmost) in `dst` with the bits in `src`. 
   // The `SrcBitcount` argument is the number of bits to replace.
   template<typename T, size_t DstBitcount = (sizeof(T) * 8), size_t SrcBitcount>
   constexpr T replace_most_significant_bits(T dst, T src, size_t bitcount) {
      constexpr auto mask  = all_ones<SrcBitcount, T>();
      constexpr auto shift = DstBitcount - SrcBitcount;

      dst &= ~(mask << shift);
      dst |= (src & mask) << shift;
      return dst;
   }
}
