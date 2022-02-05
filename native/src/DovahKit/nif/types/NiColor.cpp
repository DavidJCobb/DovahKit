#include "NiColor.h"
#include "../reader.h"

namespace nifDK {
   void NiColor::read(file_reader& reader) {
      reader.require_size(3);
      reader.unchecked_read(*this);
   }
   void NiColor::unchecked_read(file_reader& reader) {
      reader.unchecked_read(this->r);
      reader.unchecked_read(this->g);
      reader.unchecked_read(this->b);
   }

   void NiColorA::read(file_reader& reader) {
      reader.require_size(4);
      reader.unchecked_read(*this);
   }
   void NiColorA::unchecked_read(file_reader& reader) {
      reader.unchecked_read(this->r);
      reader.unchecked_read(this->g);
      reader.unchecked_read(this->b);
      reader.unchecked_read(this->a);
   }
}