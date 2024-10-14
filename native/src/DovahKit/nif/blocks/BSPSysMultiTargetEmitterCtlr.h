#pragma once
#include "NiPSysEmitterCtlr.h"

namespace nifDK::block_types {
   class BSMasterParticleSystem;

   class BSPSysMultiTargetEmitterCtlr : public NiPSysEmitterCtlr {
      public:
         static constexpr const char* const type_name = "BSPSysMultiTargetEmitterCtlr";
      public:
         uint16_t max_emitters = 0;
         BSMasterParticleSystem* master_particle_system = nullptr; // should be the root block

         virtual void parse(file_reader&) override;
   };
}