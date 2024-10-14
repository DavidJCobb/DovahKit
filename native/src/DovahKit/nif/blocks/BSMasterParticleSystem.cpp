#include "BSMasterParticleSystem.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSMasterParticleSystem::parse(file_reader& reader) {
      NiNode::parse(reader);
      
      reader.read(this->max_emitter_objects);
      {
         uint32_t count = 0;
         reader.read(count);
         this->particle_systems.resize(count);
         for (uint32_t i = 0; i < count; ++i)
            reader.read_ref(this->particle_systems[i]);
      }
   }
}