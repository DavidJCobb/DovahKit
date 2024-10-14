#include "NiPSysEmitterCtlrData.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiPSysEmitterCtlrData::parse(file_reader& reader) {
      this->birth_rate_keys.read(reader);

      uint32_t count;
      reader.read(count);
      this->active_keys.resize(count);
      for (auto& key : this->active_keys)
         key.read(reader, KeyType::linear);
   }
}