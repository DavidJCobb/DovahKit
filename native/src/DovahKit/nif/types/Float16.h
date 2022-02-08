#pragma once
#include <cmath>
#include <cstdint>

namespace nifDK {
   class file_reader;

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
      
      operator float() const;

      void read(file_reader&);
      void unchecked_read(file_reader&);
   };
}