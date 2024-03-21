#pragma once
#include <bit>
#include <cstdint>
#include <string_view>

namespace cobb::hashing {
   struct crc32_params {
      uint32_t initial_value = 0xFFFFFFFF;
      bool     invert_bits   = true;
      uint32_t polynomial    = 0xEDB88320; // LSB-first polynomial
   };

   namespace impl {
      constexpr char no_op_adjust_function(char c) noexcept {
         return c;
      }
   }

   template<crc32_params Params = crc32_params{}, auto PreAdjustCharFunctor = &impl::no_op_adjust_function>
   constexpr uint32_t crc32(std::string_view src) {
      uint32_t dst = Params.initial_value;
      for (uint8_t c : src) {
         if constexpr (PreAdjustCharFunctor != &impl::no_op_adjust_function) {
            c = PreAdjustCharFunctor(c);
         }
         for (uint8_t k = 0; k < 8; ++k) {
            uint32_t dst_bit_negated = (uint32_t)0 - ((dst ^ c) & 1);

            dst = (dst >> 1) ^ (dst_bit_negated & Params.polynomial);
            c >>= 1;
         }
      }
      if constexpr (Params.invert_bits) {
         dst ^= 0xFFFFFFFF;
      }
      return dst;
   }
}
