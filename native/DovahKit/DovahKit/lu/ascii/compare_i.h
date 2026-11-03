#pragma once
#include <string_view>

namespace lu::ascii {
   // Lexicographically compare two strings, using case-insensitivity for ASCII 
   // glyphs, and return a signed difference.
   // 
   // NOTE: You must ensure that the `lu::cpuinfo` singleton exists (call the 
   // `get_or_create` function at program startup).
   extern int compare_i_simd(const std::string_view a, const std::string_view b);

   // Lexicographically compare two strings, using case-insensitivity for ASCII 
   // glyphs, and return a signed difference.
   constexpr int compare_i(const std::string_view a, const std::string_view b) {
      if (!std::is_constant_evaluated()) {
         return compare_i_simd(a, b);
      }
      size_t size_a   = a.size();
      size_t size_b   = b.size();
      size_t size_min = (size_a < size_b) ? size_a : size_b;
      for (size_t i = 0; i < size_min; ++i) {
         unsigned char x = (unsigned char)a[i];
         unsigned char y = (unsigned char)b[i];
         if (x >= 'A' && x <= 'Z')
            x |= 0x20;
         if (y >= 'A' && y <= 'Z')
            y |= 0x20;
         if (x != y)
            return (x - y);
      }
      if (size_a != size_b) {
         if (size_a == size_min)
            return -(unsigned char)b[size_a];
         return (int)a[size_b];
      }
      return 0;
   }
}