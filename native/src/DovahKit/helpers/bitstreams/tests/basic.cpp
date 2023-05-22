#pragma once
#include "../reader.h"
#include "../writer.h"

#include <cstdint>
#include "helpers/class_array.h"

namespace {
   constexpr bool result_01 = []() -> bool {
      constexpr uintmax_t bit_pattern = (uintmax_t(0b10101010101010101010101010101010) << 32) | uintmax_t(0b10101010101010101010101010101010);

      using types_of_interest = cobb::class_array<
         uint8_t,
         uint16_t,
         uint32_t,
         uint64_t//,
      >;

      return types_of_interest::for_each_until_false<[]<typename Current>() {
         cobb::bitstreams::writer w;
         w.stream(w.header());
         w.stream((Current)bit_pattern);

         cobb::bitstreams::reader r;
         r.set_buffer(w.data(), w.get_bytespan());
         //
         Current v;
         r.stream(v);

         return v == (Current)bit_pattern;
      }>();
   }();
   static_assert(
      result_01,
      "Test for simple integer types -- write, and then read back."
   );
}

namespace {
   constexpr bool result_02 = []() -> bool {
      constexpr uintmax_t bit_pattern = (uintmax_t(0b10101010101010101010101010101010) << 32) | uintmax_t(0b10101010101010101010101010101010);

      using types_of_interest = cobb::class_array<
         uint8_t,
         uint16_t,
         uint32_t,
         uint64_t//,
      >;

      return types_of_interest::for_each_until_false<[]<typename Current>() {
         for (uint8_t offset = 1; offset < 8; ++offset) {
            cobb::bitstreams::writer w;
            w.stream(w.header());
            //
            w.stream_bits(offset, 0);
            w.stream((Current)bit_pattern);

            cobb::bitstreams::reader r;
            r.set_buffer(w.data(), w.get_bytespan());
            //
            Current v;
            int dummy;
            r.stream_bits(offset, dummy);
            r.stream(v);

            if (v != (Current)bit_pattern)
               return false;
         }
         return true;
      }>();
   }();
   static_assert(
      result_02,
      "Test for non-byte-aligned streaming, for all possible offsets from byte alignment."
   );
}