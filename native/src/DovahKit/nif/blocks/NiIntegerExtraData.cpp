#include "NiIntegerExtraData.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiIntegerExtraData::parse(file_reader& reader) {
      NiExtraData::parse(reader);
      reader.read(this->value);
   }
}