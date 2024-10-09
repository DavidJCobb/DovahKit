#include "./for_each.h"
#include <array>
#include <string>

static_assert(
   []() -> bool {
      std::string test_data = "ABCD";

      size_t codepoint_count = 0;
      cobb::utf8::for_each(
         test_data,
         [&codepoint_count](uint32_t c) -> void {
            ++codepoint_count;
         }
      );
      return codepoint_count == test_data.size();
   }(),
   "This string is ASCII-only, and the number of code points seen should exactly match the length in bytes."
);

static_assert(
   []() -> bool {
      std::string test_data = {
         (char)0xE1,
         (char)0xA0,
         (char)0xC0,
         (char)0x00
      };

      size_t codepoint_count = 0;
      cobb::utf8::for_each(
         test_data,
         [&codepoint_count](uint32_t c) -> void {
            ++codepoint_count;
         }
      );
      return codepoint_count == 2;
   }(),
   "This should test as two invalid code point representations: E1,A0 and C0. Each should be substituted with 0xFFFD."
);

static_assert(
   []() -> bool {
      std::string test_data = {
         (char)0b11101101,
         (char)0b10100000,
         (char)0b10000000
      };

      bool   is_invalid      = false;
      size_t codepoint_count = 0;
      cobb::utf8::for_each(
         test_data,
         [&is_invalid, &codepoint_count](uint32_t c) -> void {
            is_invalid = (c == 0xFFFD);
            ++codepoint_count;
         }
      );
      return is_invalid && codepoint_count == 1;
   }(),
   "This should test as an invalid code point, since the point being tested is an unpaired surrogate half (0xD800) in UTF-16."
);

static_assert(
   []() -> bool {
      std::string test_data = {
         (char)0b11101101,
         (char)0b10100000,
         (char)0b10000000
      };

      uint32_t first_codepoint = 0;
      size_t   codepoint_count = 0;
      cobb::utf8::for_each<true>(
         test_data,
         [&first_codepoint, &codepoint_count](uint32_t c) -> void {
            if (codepoint_count == 0)
               first_codepoint = c;
            ++codepoint_count;
         }
      );
      return first_codepoint == 0xD800 && codepoint_count == 1;
   }(),
   "This should test as 0xD800, since we've enabled WTF-8 mode via a template parameter."
);