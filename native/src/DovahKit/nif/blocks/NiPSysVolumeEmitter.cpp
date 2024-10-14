#include "NiPSysVolumeEmitter.h"
#include "../reader.h"
#include "NiNode.h"

namespace nifDK::block_types {
   void NiPSysVolumeEmitter::parse(file_reader& reader) {
      NiPSysEmitter::parse(reader);
      reader.read_ref(this->emitter_object);
   }
}