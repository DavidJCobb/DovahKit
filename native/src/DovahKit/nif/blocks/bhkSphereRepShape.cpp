#include "bhkSphereRepShape.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkSphereRepShape::parse(file_reader& reader) {
      reader.read(this->material);
      reader.read(this->radius);
   }
}