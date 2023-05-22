#pragma once
#include "../reader.h"
#include "../writer.h"

#include <cstdint>
#include <string>
#include <vector>

namespace {
   constexpr bool result_01 = []() -> bool {
      constexpr uintmax_t bit_pattern = (uintmax_t(0b10101010101010101010101010101010) << 32) | uintmax_t(0b10101010101010101010101010101010);

      struct test_type {
         uint8_t  field_a = 0;
         uint16_t field_b = 0;

         constexpr void stream(cobb::bitstreams::reader& s) {
            s.stream(field_a, field_b);
         }
         constexpr void stream(cobb::bitstreams::writer& s) const {
            s.stream(field_a, field_b);
         }

         constexpr bool operator==(const test_type&) const noexcept = default;
      };

      std::vector<test_type> src;
      src.push_back(test_type{ (uint8_t)bit_pattern, (uint16_t)bit_pattern });
      src.push_back(test_type{ 123, 4567 });

      cobb::bitstreams::writer w;
      w.stream(w.header());
      w.stream<3>(src);

      std::vector<test_type> dst;

      cobb::bitstreams::reader r;
      r.set_buffer(w.data(), w.get_bytespan());
      r.stream<3>(dst);

      return src == dst;
   }();
   static_assert(
      result_01,
      "Test for simple a vector of simple structs."
   );
}

namespace {
   constexpr bool result_02 = []() -> bool {
      const std::string src = "Hello, world!";

      cobb::bitstreams::writer w;
      w.stream(w.header());
      w.stream<5>(src);

      std::string dst;

      cobb::bitstreams::reader r;
      r.set_buffer(w.data(), w.get_bytespan());
      r.stream<5>(dst);

      //return src == dst; // IntelliSense chokes on this for strings longer than one char, bizarrely enough. Issue with MSVC's __builtin_memcmp?
      if (src.size() != dst.size())
         return false;
      for (size_t i = 0; i < src.size(); ++i)
         if (src[i] != dst[i])
            return false;
      return true;
   }();
   static_assert(
      result_02,
      "Test for short string."
   );
}