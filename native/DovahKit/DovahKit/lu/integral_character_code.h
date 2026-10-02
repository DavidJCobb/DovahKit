#pragma once
#include <cstddef> // size_t
#include <cstdint>
#include <type_traits>
#include "./type_aliases/uint_of_size.h"

namespace lu {
   //
   // Stores a character code in a single integral, such that the first char becomes the 
   // most significant byte, and the last char becomes the least significant byte.
   //
   template<size_t Bytecount>
   struct integral_character_code {
      public:
         static constexpr const size_t character_count = Bytecount;
         using integral_type = uint_of_size<character_count>;

      public:
         union {
            integral_type value = 0;
            char          bytes[character_count];
         };
         
         constexpr integral_character_code() {}
         constexpr integral_character_code(integral_type v) requires (character_count <= 4) : value(v) {}

         // MSVC supports four-character literals e.g. 'ABCD', but not eight-character 
         // literals; thus `eight_cc` needs a string-based constructor.
         consteval integral_character_code(const char* s) {
            this->value = 0;
            if (!s)
               return;
            uint8_t i = 0;
            for (; i < character_count; ++i) {
               if (!s[i])
                  break;
               this->value |= integral_type(s[i]) << (8 * (character_count - 1 - i));
            }
            if (std::is_constant_evaluated()) {
               if (i == character_count && s[character_count]) // don't allow inputs that are too long
                  throw;
            }
         }
         
         constexpr integral_character_code& operator=(const integral_character_code& other) noexcept {
            this->value = other.value;
            return *this;
         }

         constexpr char get_nth_character(size_t n) const noexcept {
            return this->value >> ((character_count - 1 - n) * 8);
         }
         
         constexpr operator integral_type() const noexcept { return this->value; }
   };

   using two_cc   = integral_character_code<2>;
   using four_cc  = integral_character_code<4>;
   using eight_cc = integral_character_code<8>;

   template<typename T>
   concept integral_character_code_type = requires {
      { T::character_count };
      requires std::is_same_v<T, integral_character_code<T::character_count>>;
   };
}