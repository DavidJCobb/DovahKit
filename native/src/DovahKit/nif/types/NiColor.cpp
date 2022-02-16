#include "NiColor.h"
#include "../reader.h"

#include <intrin.h>
#include "helpers/cpuinfo.h"

namespace nifDK {
   void NiColor::read(file_reader& reader) {
      reader.require_size(3 * sizeof(float));
      reader.unchecked_read(*this);
   }
   void NiColor::unchecked_read(file_reader& reader) {
      reader.unchecked_read(this->r);
      reader.unchecked_read(this->g);
      reader.unchecked_read(this->b);
   }

   void NiColorA::read(file_reader& reader) {
      reader.require_size(4 * sizeof(float));
      reader.unchecked_read(*this);
   }
   void NiColorA::unchecked_read(file_reader& reader) {
      reader.unchecked_read(this->r);
      reader.unchecked_read(this->g);
      reader.unchecked_read(this->b);
      reader.unchecked_read(this->a);
   }
   //
   void NiColorA::set_from_bytes(uint8_t values[4]) {
      this->r = (float)values[0] / 255.0F;
      this->g = (float)values[1] / 255.0F;
      this->b = (float)values[2] / 255.0F;
      this->a = (float)values[3] / 255.0F;
   }
}