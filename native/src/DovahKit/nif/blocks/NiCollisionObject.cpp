#include "NiCollisionObject.h"
#include "../reader.h"

#include "NiAVObject.h"

namespace nifDK::block_types {
   void NiCollisionObject::parse(file_reader& reader) {
      reader.read_ref(this->owner);
   }
}