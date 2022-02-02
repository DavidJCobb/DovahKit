#include "BSShaderTextureSet.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSShaderTextureSet::parse(file_reader& reader) {
      int32_t count;
      reader.read(count);
      for (int i = 0; i < (std::min)(count, (int32_t)supported_texture_count); ++i) {
         reader.read_prefixed_string<uint32_t>(this->textures.list[i]);
      }
   }
}