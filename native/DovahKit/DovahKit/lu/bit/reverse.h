#pragma once
#include <concepts>
#include <cstddef> // size_t
#include <cstdint>
#include <type_traits>

namespace lu::bits {
   template<std::integral T>
      requires std::is_unsigned_v<T>
   constexpr auto reverse(T v) {
      if constexpr (std::is_same_v<T, bool>) {
         return v;
      } else {
         constexpr const size_t bitcount = sizeof(T) * 8;
         T out = 0;
         for (size_t i = 0; i < bitcount; ++i) {
            const auto bit = (v >> i) & 1;
            out |= (bit << (bitcount - i - 1));
         }
         return out;
      }
   }

   // Per: <https://stackoverflow.com/a/2602885>
   template<>
   constexpr auto reverse<uint8_t>(uint8_t v) {
      v = (v & 0xF0) >> 4 | (v & 0x0F) << 4;
      v = (v & 0xCC) >> 2 | (v & 0x33) << 2;
      v = (v & 0xAA) >> 1 | (v & 0x55) << 1;
      return v;
   }

   template<>
   constexpr auto reverse<uint16_t>(uint16_t v) {
      v = (v & 0xFF00) >> 8 | (v & 0x00FF) << 8;
      v = (v & 0xF0F0) >> 4 | (v & 0x0F0F) << 4;
      v = (v & 0xCCCC) >> 2 | (v & 0x3333) << 2;
      v = (v & 0xAAAA) >> 1 | (v & 0x5555) << 1;
      return v;
   }

   template<>
   constexpr auto reverse<uint32_t>(uint32_t v) {
      v = (v & 0xFFFF0000) >> 16 | (v & 0x0000FFFF) << 16;
      v = (v & 0xFF00FF00) >>  8 | (v & 0x00FF00FF) <<  8;
      v = (v & 0xF0F0F0F0) >>  4 | (v & 0x0F0F0F0F) <<  4;
      v = (v & 0xCCCCCCCC) >>  2 | (v & 0x33333333) <<  2;
      v = (v & 0xAAAAAAAA) >>  1 | (v & 0x55555555) <<  1;
      return v;
   }
   
   // Alternate overload of the function for bitcounts that aren't exactly equal to 
   // the width of the value type (e.g. reversing 15-bit, 3-bit, etc., values).
   template<std::integral T>
      requires std::is_unsigned_v<T>
   constexpr auto reverse(T v, size_t bitcount) {
      T out = 0;
      for (size_t i = 0; i < bitcount; ++i) {
         const auto bit = (v >> i) & 1;
         out |= (bit << (bitcount - i - 1));
      }
      return out;
   }

   // Alternate version of the function for bitcounts that aren't exactly equal to 
   // the width of the value type (e.g. reversing 15-bit, 3-bit, etc., values)., 
   // when the bitcount is statically known.
   template<std::integral T, size_t Bitcount>
      requires std::is_unsigned_v<T>
   constexpr auto reverse_with_bitcount(T v) {
      T out = 0;
      for (size_t i = 0; i < Bitcount; ++i) {
         const T bit = (v >> i) & 1;
         out |= (bit << (Bitcount - i - 1));
      }
      return out;
   }
}