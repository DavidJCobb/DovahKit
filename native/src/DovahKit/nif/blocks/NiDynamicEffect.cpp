#include "NiDynamicEffect.h"
#include "../reader.h"

#include "NiNode.h"

namespace nifDK::block_types {
   void NiDynamicEffect::parse(file_reader& reader) {
      NiAVObject::parse(reader);
      //
      if (reader.version() >= file_version::from_parts<10, 1, 0, 106> && reader.user_version<2>() < 130)
         reader.read(this->enabled);
      //
      bool is_old = reader.version() <= file_version::from_parts<4, 0, 0, 2>;
      bool is_new = reader.version() >= file_version::from_parts<10, 1, 0, 0> && reader.user_version<2>() < 130;
      if (is_old || is_new) {
         uint32_t count;
         reader.read(count);
         if (is_new || reader.version() <= file_version::from_parts<3, 3, 0, 13>) {
            this->targets.resize(count);
            for (auto& t : this->targets)
               reader.read_ref(t);
         }
      }
   }
}