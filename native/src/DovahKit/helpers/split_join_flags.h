#pragma once
#include <type_traits>

namespace cobb {
   template<typename MaskType, typename... Args> requires (std::is_same_v<Args, bool> && ...)
   [[nodiscard]] constexpr MaskType join_flags(Args... args) {
      MaskType i   = 0;
      MaskType out = 0;
      ((
         args ? (out |= (1 << i++)) : (i++)
      ), ...);
      return out;
   }
   static_assert(join_flags<uint16_t>(true, true,  true, false) == 0b0111);
   static_assert(join_flags<uint16_t>(true, false, true, false) == 0b0101);

   template<typename MaskType, typename... Args> requires (std::is_same_v<Args, bool> && ...)
   constexpr void split_flags(MaskType mask, Args&... args) {
      MaskType i = 0;
      ((args = (mask & (1 << i++)) != 0), ...);
   }
   static_assert([]() -> bool {
      bool a = true;
      bool b = false;
      bool c = true;
      bool d = false;
      split_flags<uint16_t>(0b0101, a, b, c, d);
      return a && !b && c && !d;
   }());
}
