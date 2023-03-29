#pragma once
#include "./all_ones.h"

namespace cobb::bits {
   //
   // Given a broader bitfield `dst`, overwrites `bitcount` bits located `left_shift_by` from the 
   // righthand edge with `src`. For example:
   // 
   //    replace_bits(0b00000000, 0b111, 3, 4); // 0b01110000
   //
   template<typename T> constexpr T replace_bits_at_index(T dst, T src, size_t bitcount, size_t left_shift_by) {
      auto mask = all_ones<T>(bitcount);

      dst &= ~(mask << left_shift_by);
      dst |= (src & mask) << left_shift_by;
      return dst;
   }
}
