#include "BSInvMarker.h"
#include "../reader.h"
#include "../types/Float16.h"

namespace nifDK::block_types {
   void BSInvMarker::parse(file_reader& reader) {
      NiExtraData::parse(reader);
      //
      std::array<uint16_t, 3> radians;
      reader.read(radians);
      reader.read(this->zoom);
      //
      for (int i = 0; i < radians.size(); ++i) {
         this->rotation[i] = (float)radians[i] / 1000.0F;
      }
   }
}