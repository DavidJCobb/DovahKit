#pragma once
#include <string_view>

namespace cobb {
   // check if two strings are equal, using case-insensitivity for ASCII glyphs
   extern int strieq_ascii_simd(std::string_view a, std::string_view b);

   constexpr int strieq_ascii(std::string_view a, std::string_view b) {
      if (!std::is_constant_evaluated()) {
         return strieq_ascii_simd(a, b);
      }
      size_t size = a.size();
      if (size != b.size())
         return false;
      for (size_t i = 0; i < size; ++i) {
         char x = a[i];
         char y = b[i];
         if (x >= 'A' && x <= 'Z')
            x |= 0x20;
         if (y >= 'A' && y <= 'Z')
            y |= 0x20;
         if (x != y)
            return false;
      }
      return true;
   }
}