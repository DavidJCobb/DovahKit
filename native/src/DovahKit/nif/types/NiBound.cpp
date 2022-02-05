#include "NiBound.h"
#include "../reader.h"

namespace nifDK {
   void NiBound::read(file_reader& reader) {
      reader.require_size(sizeof(float) * 4);
      reader.unchecked_read(*this);
   }
   void NiBound::unchecked_read(file_reader& reader) {
      reader.unchecked_read(this->center);
      reader.unchecked_read(this->radius);
   }
}