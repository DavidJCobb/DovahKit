#include "bhkSimpleShapePhantom.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkSimpleShapePhantom::parse(file_reader& reader) {
      bhkWorldObject::parse(reader);
      reader.read(this->pad00);
      reader.read(this->transform);
   }
}