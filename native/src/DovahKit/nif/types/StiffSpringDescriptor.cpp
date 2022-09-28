#include "StiffSpringDescriptor.h"
#include "../reader.h"

namespace nifDK {
   void StiffSpringDescriptor::read(file_reader& reader) {
      reader.read(this->pivot_a);
      reader.read(this->pivot_b);
      reader.read(this->length);
   }
}