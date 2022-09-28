#include "bhkMultiSphereShape.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkMultiSphereShape::parse(file_reader& reader) {
      bhkSphereRepShape::parse(reader);
      reader.read(this->unknown_1);
      reader.read(this->unknown_2);

      uint32_t count;
      reader.read(count);
      this->spheres.resize(count);
      reader.read_vector_contents(this->spheres);
   }
}