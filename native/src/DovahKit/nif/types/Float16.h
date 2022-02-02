#pragma once
#include <cmath>
#include <cstdint>

namespace nifDK {
   struct Float16 {
      static constexpr int fraction_bits = 10;
      static constexpr int exponent_bits =  5;

      static constexpr uint16_t fraction_mask = ((uint16_t(1) << fraction_bits) - 1);
      static constexpr uint16_t exponent_mask = ((uint16_t(1) << exponent_bits) - 1);

      static constexpr uint16_t infinity_pos = 0b0111110000000000;
      static constexpr uint16_t infinity_neg = 0b1111110000000000;
      static constexpr uint16_t nan_quiet    = 0b0111111111111111;
      static constexpr uint16_t nan_signal   = 0b1111111111111111;

      uint16_t value = 0;

      constexpr Float16() {}
      constexpr Float16(uint16_t d) : value(d) {}
      
      operator float() const {
         auto exp  = ((int32_t)value >> fraction_bits) & exponent_mask;
         auto frac = (uint32_t)value & fraction_mask;
         if (exp == 0b11111) {
            if (frac == 0)
               return (value & (1 << 15)) ? -INFINITY : INFINITY;
            return NAN;
         }
         float offset = 0;
         if (exp == 0b00000) {
            exp = -14;
         } else {
            offset = 1;
            exp -= 15;
         }
         //
         float data = offset + (float)frac / 1024;
         data *= std::pow(2.0F, exp);
         if (value & (uint16_t(1) << 15))
            data = -data;
         return data;
      }
   };
}