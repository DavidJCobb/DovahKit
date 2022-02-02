#include "NiShadeProperty.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiShadeProperty::parse(file_reader& reader) {
      NiProperty::parse(reader);
      //
      if (reader.user_version<2>() <= 34) {
         reader.read(this->phong_shading);
      }
   }
}