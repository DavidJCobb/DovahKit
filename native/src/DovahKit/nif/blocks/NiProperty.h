#pragma once
#include "NiObjectNET.h"

namespace nifDK::block_types {
   class NiProperty : public NiObjectNET {
      public:
         static constexpr const char* const type_name = "NiProperty";
   };
}