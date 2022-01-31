#include "NiAVObject.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiAVObject::parse(file_reader& reader) {
      NiObjectNET::parse(reader);
      reader.read(this->flags);
      // unknown short?
      reader.read(this->transform);
      reader.read_ref(this->collision);
   }
}