#include "bhkConvexSweepShape.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkConvexSweepShape::parse(file_reader& reader) {
      reader.read_ref(this->subject);
      reader.read(this->material);
      reader.read(this->radius);
      reader.read(this->unknown);
   }
}