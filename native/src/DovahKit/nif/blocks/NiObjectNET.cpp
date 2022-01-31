#include "NiObjectNET.h""
#include "../reader.h"

namespace nifDK::block_types {
   void NiObjectNET::parse(file_reader& reader) {
      // shader type?
      reader.read_prefixed_string<uint32_t>(this->name);
      reader.read(this->extra);
      reader.read_ref(this->controller);
   }
}