#pragma once
#include "NiTimeController.h"

namespace nifDK::block_types {
   class NiInterpController : public NiTimeController {
      public:
         static constexpr const char* const type_name = "NiInterpController";
   };
}