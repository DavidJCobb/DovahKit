#include "BSPSysMultiTargetEmitterCtlr.h"
#include "BSMasterParticleSystem.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSPSysMultiTargetEmitterCtlr::parse(file_reader& reader) {
      NiPSysEmitterCtlr::parse(reader);
      reader.read(this->max_emitters);
      reader.read_ref(this->master_particle_system);
   }
}