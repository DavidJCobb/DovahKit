#include "NiPSysModifier.h"
#include "../reader.h"
#include "NiParticleSystem.h"

namespace nifDK::block_types {
   void NiPSysModifier::parse(file_reader& reader) {
      reader.read_indexed_string(this->name);
      reader.read(this->order);
      reader.read_ref(this->target);
      reader.read(this->active);
   }
}