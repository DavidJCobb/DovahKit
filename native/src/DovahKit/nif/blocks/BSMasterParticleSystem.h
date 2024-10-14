#pragma once
#include "NiNode.h"

namespace nifDK::block_types {
   class NiDynamicEffect;

   class BSMasterParticleSystem : public NiNode {
      public:
         static constexpr const char* const type_name = "BSMasterParticleSystem";
      public:
         uint16_t max_emitter_objects = 0;
         std::vector<NiAVObject*> particle_systems;

         virtual void parse(file_reader&) override;
   };
}