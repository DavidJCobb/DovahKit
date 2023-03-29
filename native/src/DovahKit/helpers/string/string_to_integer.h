#pragma once
#include <type_traits>

namespace cobb {
   namespace impl::_string_to_integer {
      template<typename Integer = int, size_t Base = 10> requires (std::is_arithmetic_v<Integer> && (Base <= 36))
      constexpr Integer char_to_integer(char c, bool& ok) {
         Integer digit = c - '0';
         if constexpr (Base <= 10) {
            if (digit >= Base) {
               ok = false;
               return {};
            }
         }
         if (digit < Base) {
            return digit;
         }
         if constexpr (Base > 10) {
            //
            // For bases above 10, move on to using letters.
            //
            if (c >= 'a' && c <= 'z')
               c -= 0x20;
            digit  = c - 'a';
            if (digit < 26) {
               digit += 10;
               if (digit >= Base) {
                  ok = false;
                  return {};
               }
               return digit;
            }
            //
            // Not a letter. Fall through.
            //
         }
         //
         // Unrecognized character:
         //
         ok = false;
         return {};
      }
   }

   template<typename Integer, size_t Base = 10> requires (std::is_arithmetic_v<Integer> && (Base <= 36))
   constexpr Integer string_to_integer(const char* str, bool* ok) {
      Integer out = {};
      bool    sgn = false;
      if constexpr (std::is_signed_v<Integer>) {
         if (*str == '-') {
            sgn = true;
            ++str;
         }
      }
      for (; *str; ++str) {
         char c = *str;

         bool c_ok;
         auto digit = impl::_string_to_integer::char_to_integer(c, c_ok);
         if (!c_ok) {
            if (ok)
               *ok = false;
            return {};
         }
         out *= Base;
         out += digit;
      }
      if constexpr (std::is_signed_v<Integer>) {
         if (sgn)
            out *= -1;
      }
      if (ok)
         *ok = true;
      return out;
   }
   
   template<typename Integer, size_t Base = 10> requires (std::is_arithmetic_v<Integer> && (Base <= 36))
   constexpr Integer string_to_integer(const char* str, size_t len, bool* ok) {
      Integer out = {};
      bool    sgn = false;
      size_t  i   = 0;
      if constexpr (std::is_signed_v<Integer>) {
         if (len && str[0] == '-') {
            sgn = true;
            ++i;
         }
      }
      for (; i < len; ++i) {
         char c = str[i];
         
         bool c_ok;
         auto digit = impl::_string_to_integer::char_to_integer(c, c_ok);
         if (!c_ok) {
            if (ok)
               *ok = false;
            return {};
         }
         out *= Base;
         out += digit;
      }
      if constexpr (std::is_signed_v<Integer>) {
         if (sgn)
            out *= -1;
      }
      if (ok)
         *ok = true;
      return out;
   }
}