#pragma once
#include <bit>
#include <cstdint>
#include <type_traits>

namespace cobb {
   template<typename T> requires (std::is_floating_point_v<T>)
   constexpr void to_ratio(T input, int64_t& out_numer, uint64_t& out_denom) {
      //
      // Per: https://stackoverflow.com/questions/50962041/how-can-i-get-numerator-and-denominator-from-a-fractional-number
      //

      constexpr const size_t bitcount          = 32;
      constexpr const size_t bits_for_exponent = 8;

      constexpr const size_t exponent_bitshift = bitcount - 1 - bits_for_exponent;
      constexpr const size_t exponent_all_set  = (1 << bits_for_exponent) - 1;

      std::uint32_t bits = std::bit_cast<uint32_t, float>(input);

      bool     is_signed = bits >> (bitcount - 1);
      uint32_t exponent_bits = (bits >> exponent_bitshift) & exponent_all_set;
      uint32_t mantissa_bits = (bits & ((1 << exponent_bitshift) - 1));
      if (exponent_bits == 0 && mantissa_bits == 0) { // +0, -0
         out_numer = 0;
         out_denom = 1;
         return;
      } else if (exponent_bits == exponent_all_set && mantissa_bits == 0) { // +INF, -INF
         out_numer = is_signed ? -1 : 1;
         out_denom = 0;
         return;
      } else if (exponent_bits == exponent_all_set) { // NaN
         out_numer = 0;
         out_denom = 0;
         return;
      }

      uint32_t significand = (uint32_t)(1 << 23) | mantissa_bits;

      uint32_t significand_zeroes = std::countr_zero(significand);
      significand >>= significand_zeroes;

      int32_t exponent = (int32_t)exponent_bits - 127 - 23 + significand_zeroes;
      if (exponent < 0) {
         out_numer = significand;
         out_denom = 1 << -exponent;
      } else {
         out_numer = (significand * (1 << exponent));
         out_denom = 1;
      }
      if (is_signed)
         out_numer = -out_numer;
   }
}