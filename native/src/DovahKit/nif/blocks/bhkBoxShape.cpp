#include "bhkBoxShape.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkBoxShape::parse(file_reader& reader) {
      bhkConvexShape::parse(reader);
      reader.read(this->pad00);
      reader.read(this->dimensions);
   }
}