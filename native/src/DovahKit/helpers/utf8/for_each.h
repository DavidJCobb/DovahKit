#pragma once
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace cobb::utf8 {
   //
   // Loops over every byte in a single-byte string view, and invokes the passed-in 
   // functor on each code point seen. Invalid code point representations are passed 
   // to the functor as 0xFFFD. If the functor returns bool, then returning false will 
   // halt the loop early.
   //
   template<
      bool WTF8 = false,
      typename Functor
   >
   constexpr void for_each(std::string_view src, Functor&& functor) {
      constexpr const std::uint32_t invalid_codepoint_substitution = 0xFFFD;

      using functor_return_type = std::invoke_result_t<Functor, std::uint32_t>;
      constexpr const bool functor_returns_bool = std::is_same_v<functor_return_type, bool>;

      for (size_t i = 0; i < src.size(); ++i) {
         std::uint32_t codepoint = src[i];
         if (!(codepoint & 0b10000000)) {
            if constexpr (functor_returns_bool) {
               bool proceed = functor(codepoint);
               if (!proceed)
                  return;
            } else {
               functor(codepoint);
            }
            continue;
         }

         //
         // Character is UTF-8-encoded.
         //

         size_t consume = 0;
         if ((codepoint & 0b111'00000) == 0b110'00000) {
            consume    = 1;
            codepoint &= 0b00011111;
         } else if ((codepoint & 0b1111'0000) == 0b1110'0000) {
            consume    = 2;
            codepoint &= 0b00001111;
         } else if ((codepoint & 0b11111'000) == 0b11110'000) {
            consume    = 3;
            codepoint &= 0b00000111;
         }

         for (size_t j = 1; j <= consume; ++j) {
            char c = src[i + j];
            if ((c & 0b11'000000) != 0b10'000000) {
               //
               // UTF-8 multi-byte code point is truncated!
               //
               codepoint = invalid_codepoint_substitution;
               consume = j;
               break;
            }
            codepoint <<= 6;
            codepoint |= (c & 0b00'111111);
         }
         
         if (codepoint != invalid_codepoint_substitution) {
            //
            // Code point is "basically" valid, but could still be non-canonical.
            //
            if constexpr (!WTF8) {
               if (codepoint >= 0xD800 && codepoint <= 0xDFFF) // Illegal: unpaired surrogate halves in UTF-16; ergo banned in UTF-8.
                  codepoint = invalid_codepoint_substitution;
            }
            if (
               codepoint < 0b01111111 // Illegal: 7-bit character used an overlong encoding, e.g 0xC080 for 0x00.
            || codepoint > 0x110000   // Illegal: not representable in UTF-16; ergo banned in UTF-8
            ) {
               codepoint = invalid_codepoint_substitution;
            }
         }

         if constexpr (functor_returns_bool) {
            bool proceed = functor(codepoint);
            if (!proceed)
               return;
         } else {
            functor(codepoint);
         }
         i += consume;
      }
   }
}
