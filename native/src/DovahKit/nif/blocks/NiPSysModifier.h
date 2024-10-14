#pragma once
#include "NiObject.h"

namespace nifDK::block_types {
   class NiParticleSystem;

   class NiPSysModifier : public NiObject {
      public:
         static constexpr const char* const type_name = "NiPSysModifier";
      public:
         std::string name;
         uint32_t    order = 0;
         NiParticleSystem* target = nullptr;
         bool active = true;

         virtual void parse(file_reader&) override;
   };
}