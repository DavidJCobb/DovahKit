#pragma once
#include <bit>
#include <cstdint>
#include <string_view>
#include "helpers/compile_time_strings/cs.h"

namespace dovah {
   class story_event_member_id {
      protected:
         // Story manager event codes are serialized as char[2], e.g. 'R1' is {0x52,0x31}. 
         // We represent them in native endianness i.e. whatever you get when you use 'R1' 
         // as an integer literal. It is assumed that any uint16_t you feed into this 
         // struct is already represented accordingly.
         uint16_t raw = 0;

         constexpr void _replace_lo_char(char c) {
            this->raw &= 0xFF00;
            this->raw |= c;
         }
         constexpr void _replace_hi_char(char c) {
            this->raw &= 0x00FF;
            this->raw |= (uint16_t)c << 8;
         }

      public:
         constexpr story_event_member_id() {}
         constexpr story_event_member_id(uint16_t v) : raw(v) {}

         constexpr operator uint16_t() const noexcept { return this->raw; }

         constexpr char get_first_char() const noexcept {
            if constexpr (std::endian::native == std::endian::little)
               return this->raw >> 8;
            else
               return this->raw;
         }
         constexpr char get_second_char() const noexcept {
            if constexpr (std::endian::native == std::endian::little)
               return this->raw;
            else
               return this->raw >> 8;
         }

         constexpr void set_first_char(char c) noexcept {
            if constexpr (std::endian::native == std::endian::little)
               _replace_hi_char(c);
            else
               _replace_lo_char(c);
         }
         constexpr void set_second_char(char c) noexcept {
            if constexpr (std::endian::native == std::endian::little)
               _replace_lo_char(c);
            else
               _replace_hi_char(c);
         }
         constexpr void set_chars(char a, char b) noexcept {
            if constexpr (std::endian::native == std::endian::little)
               this->raw = b | ((uint16_t)a << 8);
            else
               this->raw = a | ((uint16_t)b << 8);
         }

         constexpr bool operator==(const story_event_member_id&) const noexcept = default;
         constexpr bool operator==(std::string_view s) const noexcept {
            if (!this->raw)
               return s.empty();
            char a = get_first_char();
            char b = get_second_char();
            if (s[0] != a)
               return false;
            if (b) {
               if (s.size() != 2)
                  return false;
               if (s[1] != b)
                  return false;
            } else {
               if (s.size() != 1)
                  return false;
            }
            return true;
         }

         constexpr cobb::cs<3> to_string() const noexcept {
            cobb::cs<3> str;
            str[0] = get_first_char();
            str[1] = get_second_char();
            str[2] = '\0';
            return str;
         }

         constexpr void from_string(std::string_view v) noexcept {
            if (v.empty()) {
               this->raw = 0;
               return;
            }
            char a = v[0];
            char b = v.size() > 1 ? v[1] : 0;
            this->set_chars(a, b);
         }
         constexpr void from_string(const char* v) noexcept {
            if (!v || !v[0]) {
               this->raw = 0;
               return;
            }
            this->set_chars(v[0], v[1]);
         }
   };
}