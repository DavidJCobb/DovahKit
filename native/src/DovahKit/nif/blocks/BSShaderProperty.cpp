#include "BSShaderProperty.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSShaderProperty::parse(file_reader& reader) {
      NiShadeProperty::parse(reader);
      //
      if (reader.user_version<2>() <= 34) {
         reader.read(this->legacy_data.type);
         reader.read(this->legacy_data.shader_flags);
         reader.read(this->legacy_data.env_map_scale);
      }
   }
}