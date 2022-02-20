#pragma once
#include "NiInterpController.h"

namespace nifDK::block_types {
   class NiInterpolator;

   class NiSingleInterpController : public NiInterpController {
      public:
         static constexpr const char* const type_name = "NiSingleInterpController";
      public:
         NiInterpolator* interpolator = nullptr;

         virtual void parse(file_reader&) override;
   };
}