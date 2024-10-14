#pragma once
#include <vector>
#include "NiParticles.h"
#include "../types/BSVertexDesc.h"

namespace nifDK::block_types {
   class NiPSysData;
   class NiPSysModifier;

   class NiParticleSystem : public NiParticles {
      public:
         static constexpr const char* const type_name = "NiParticleSystem";
      public:
         struct {
            struct {
               uint16_t begin = 0;
               uint16_t end   = 0;
            } near;
            struct {
               uint16_t begin = 0;
               uint16_t end   = 0;
            } far;
         } distances;
         NiPSysData* data = nullptr;
         bool is_world_space = false;
         std::vector<NiPSysModifier*> modifiers;

         virtual void parse(file_reader&) override;
   };
}