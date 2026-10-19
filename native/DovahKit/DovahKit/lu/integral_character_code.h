#pragma once
#include <cstddef> // size_t
#include <cstdint>
#include <type_traits>
#include "./type_aliases/uint_of_size.h"
#include "./utils/integral_wrapper.h"

namespace lu {
   //
   // Stores a character code in a single integral, such that the first char becomes the 
   // most significant byte, and the last char becomes the least significant byte.
   //
   template<size_t Bytecount>
   struct integral_character_code : public utils::integral_wrapper<integral_character_code<Bytecount>, uint_of_size<Bytecount>> {
      protected:
         using _my_base = utils::integral_wrapper<integral_character_code, uint_of_size<Bytecount>>;

      public:
         static constexpr const size_t character_count = Bytecount;
         using integral_type = _my_base::integral_type;

      public:
         constexpr integral_character_code() {}
         constexpr integral_character_code(integral_type v) {
            this->wrapped_integral = v;
         }

         // MSVC supports four-character literals e.g. 'ABCD', but not eight-character 
         // literals; thus `eight_cc` needs a string-based constructor.
         consteval integral_character_code(const char(&s)[character_count + 1]) {
            this->wrapped_integral = 0;
            uint8_t i = 0;
            for (; i < character_count; ++i) {
               if (!s[i])
                  break;
               this->wrapped_integral |= integral_type(s[i]) << (8 * (character_count - 1 - i));
            }
            if (std::is_constant_evaluated()) {
               if (i == character_count && s[character_count]) // don't allow inputs that are too long
                  throw;
            }
         }
         
         constexpr integral_character_code& operator=(const integral_character_code& other) noexcept {
            this->wrapped_integral = other.wrapped_integral;
            return *this;
         }

         constexpr bool operator==(const char(&v)[character_count + 1]) const noexcept {
            integral_type me = this->wrapped_integral;
            for (size_t i = 0; i < character_count; ++i) {
               char c = me & 0xFF;
               char d = v[character_count - 1 - i];
               if (c != d)
                  return false;
               me >>= 8;
            }
            return v[character_count] == '\0';
         }

         constexpr char get_nth_character(size_t n) const noexcept {
            return this->wrapped_integral >> ((character_count - 1 - n) * 8);
         }
         
         constexpr operator integral_type() const noexcept { return this->wrapped_integral; }
   };

   using two_cc   = integral_character_code<2>;
   using four_cc  = integral_character_code<4>;
   using eight_cc = integral_character_code<8>;
   static_assert(sizeof(two_cc) == 2);
   static_assert(sizeof(four_cc) == 4);
   static_assert(sizeof(eight_cc) == 8);

   template<typename T>
   concept integral_character_code_type = requires {
      { T::character_count };
      requires std::is_same_v<T, integral_character_code<T::character_count>>;
   };
}