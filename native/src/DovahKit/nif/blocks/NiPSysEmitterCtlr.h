#pragma once
#include "NiPSysModifierCtlr.h"

namespace nifDK::block_types {
   class NiPSysEmitterCtlrData;

   class NiPSysEmitterCtlr : public NiPSysModifierCtlr {
      public:
         static constexpr const char* const type_name = "NiPSysEmitterCtlr";
      public:
         NiInterpolator*        visibility_interpolator = nullptr;
         NiPSysEmitterCtlrData* deprecated_data         = nullptr;

         virtual void parse(file_reader&) override;
   };
}