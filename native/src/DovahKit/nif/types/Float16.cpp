#include "Float16.h"
#include "../reader.h"

namespace nifDK {
   Float16::operator float() const {
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

   void Float16::read(file_reader& reader) {
      reader.read(this->value);
   }
   void Float16::unchecked_read(file_reader& reader) {
      reader.unchecked_read(this->value);
   }
}