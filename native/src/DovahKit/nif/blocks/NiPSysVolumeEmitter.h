#pragma once
#include "NiPSysEmitter.h"

namespace nifDK::block_types {
   class NiNode;

   class NiPSysVolumeEmitter : public NiPSysEmitter {
      public:
         static constexpr const char* const type_name = "NiPSysVolumeEmitter";
      public:
         NiNode* emitter_object = nullptr;

         virtual void parse(file_reader&) override;
   };
}