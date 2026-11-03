#pragma once
#include <string>
#include <string_view>
#include "./compare_i.h"

namespace lu::ascii {
   // Case-insensitive char traits over the ASCII charset only.
   class char_traits_ci : std::char_traits<char> {
      public:
         static constexpr int compare(const char_type* a, const char_type* b, std::size_t size) {
            return compare_i(std::string_view(a, size), std::string_view(b, size));
         }
         static constexpr const char_type* find(const char_type* ptr, std::size_t count, const char_type& ch) {
            char charset[3] = { ch, ch, '\0' };
            if (ch >= 'A' && ch <= 'Z')
               charset[1] = ch | 0x20;
            else if (ch >= 'a' && ch <= 'z')
               charset[0] = ch & ~0x20;
            auto i = std::string_view(ptr, count).find_first_of(charset);
            if (i == std::string::npos)
               return nullptr;
            return ptr + i;
         }

         static constexpr bool eq(char_type a, char_type b) {
            if (a >= 'A' && a <= 'Z')
               a |= 0x20;
            if (b >= 'A' && b <= 'Z')
               b |= 0x20;
            return a == b;
         }
         static constexpr bool lt(char_type a, char_type b) {
            if (a >= 'A' && a <= 'Z')
               a |= 0x20;
            if (b >= 'A' && b <= 'Z')
               b |= 0x20;
            return a < b;
         }
         
         static constexpr bool eq_int_type(int_type a, int_type b) {
            if (a >= 'A' && a <= 'Z')
               a |= 0x20;
            if (b >= 'A' && b <= 'Z')
               b |= 0x20;
            return a == b;
         }
   };
}