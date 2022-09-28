#include "bhkCapsuleShape.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkCapsuleShape::parse(file_reader& reader) {
      bhkConvexShape::parse(reader);
      reader.read(this->pad00);
      for (auto& item : this->endcaps) {
         reader.read(item.pos);
         reader.read(item.radius);
      }
   }
}