#pragma once
#include "NiSingleInterpController.h"

namespace nifDK::block_types {
   class NiFloatInterpController : public NiSingleInterpController {
      public:
         static constexpr const char* const type_name = "NiFloatInterpController";
   };
}