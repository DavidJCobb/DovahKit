#pragma once
#include <type_traits>

namespace lu {
   template<typename T> requires (std::is_integral_v<T> && !std::is_same_v<T, bool>)
   constexpr void edit_bit(T& dst, T mask, bool clear_or_set) {
      if (clear_or_set)
         dst |= mask;
      else
         dst &= ~mask;
   }
}
