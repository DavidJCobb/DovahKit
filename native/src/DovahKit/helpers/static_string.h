#pragma once
#include <string>

namespace cobb {
   template<char... c> struct static_string {
      const char s[sizeof...(c)] = { c... };

      consteval size_t size() const noexcept { return sizeof...(c); }
      consteval const char* data() const noexcept { return s; }
      consteval const std::string string() const noexcept { return s; }
   };

   template<uint32_t sig> using string_from_four_cc = static_string<
      (char)(sig >> 0x18),
      (char)(sig >> 0x10),
      (char)(sig >> 0x08),
      (char)sig,
      0
   >;
}