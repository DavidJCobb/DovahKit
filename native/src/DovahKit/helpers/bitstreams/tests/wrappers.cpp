#pragma once
#include "../reader.h"
#include "../writer.h"

#include <cstdint>
#include "../override_bitcount.h"

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

      constexpr const size_t test_bitcount = 7;

      test_type src;
      src.field_a = bit_pattern;
      src.field_b = bit_pattern & ((1 << test_bitcount) - 1);

      cobb::bitstreams::writer w;
      w.stream(w.header());
      w.stream(
         src.field_a,
         cobb::bitstreams::override_bitcount<test_bitcount>::of(src.field_b)
      );

      test_type dst;

      cobb::bitstreams::reader r;
      r.set_buffer(w.data(), w.get_bytespan());
      r.stream(
         dst.field_a,
         cobb::bitstreams::override_bitcount<test_bitcount>::of(dst.field_b)
      );

      return src == dst;
   }();
   static_assert(
      result_01,
      "Test for override_bitcount::of."
   );
}