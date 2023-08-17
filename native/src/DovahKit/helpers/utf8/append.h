#pragma once
#include <algorithm>
#include <cstdint>
#include <type_traits>

namespace cobb::utf8 {
   template<bool UseRFC3629Limits = true, typename String>
   constexpr void append(String& dst, std::uint32_t codepoint) {
      if (codepoint < 0b10000000) {
         dst += (char)codepoint;
         return;
      }

      if constexpr (UseRFC3629Limits) {
         if (codepoint > 0x10FFFF) {
            append(dst, 0xFFFD);
            return;
         }
      }

      std::uint8_t bytes = 1;
      for (; codepoint > 0b00'111111; codepoint >>= 6) {
         char part = (codepoint & 0b00'111111) | 0b10'000000;
         dst += part;
         //
         ++bytes;
      }

      char last   = codepoint;
      char prefix = 0;
      for(auto b = bytes; b; --b) {
         prefix >>= 1;
         prefix |= 0b10000000;
      }
      if (std::is_constant_evaluated()) {
         unsigned char impossible = ((prefix >> 1) | 0b10000000);
         if (last & impossible)
            throw;
      }
      last |= prefix;
      dst += last;

      //
      // For the above code, we appended the bytes in reverse order. Let's 
      // flip 'em around now.
      //
      std::reverse(dst.end() - bytes, dst.end());
   }
}
