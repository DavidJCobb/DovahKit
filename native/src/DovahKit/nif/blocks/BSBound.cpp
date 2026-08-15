#include "BSBound.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSBound::parse(file_reader& reader) {
      NiExtraData::parse(reader);
      //
      reader.read(this->center);
      reader.read(this->halfwidths);
   }
}