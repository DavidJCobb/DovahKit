#include "BallAndSocketDescriptor.h"
#include "../reader.h"

namespace nifDK {
   void BallAndSocketDescriptor::read(file_reader& reader) {
      reader.read(this->pivot_a);
      reader.read(this->pivot_b);
   }
}