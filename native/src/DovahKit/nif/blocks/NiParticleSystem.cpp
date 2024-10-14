#include "NiParticleSystem.h"
#include "NiPSysData.h"
#include "NiPSysModifier.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiParticleSystem::parse(file_reader& reader) {
      NiParticles::parse(reader);
      if (reader.user_version<2>() >= 83) {
         reader.read(this->distances.far.begin);
         reader.read(this->distances.far.end);
         reader.read(this->distances.near.begin);
         reader.read(this->distances.near.end);
         if (reader.user_version<2>() >= 100) {
            reader.read_ref(this->data);
         }
      }
      reader.read(this->is_world_space);
      {
         uint32_t count;
         reader.read(count);
         this->modifiers.resize(count);
         for (auto& item : this->modifiers)
            reader.read_ref(item);
      }
   }
}