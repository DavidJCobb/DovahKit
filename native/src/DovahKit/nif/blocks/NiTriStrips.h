#pragma once
#include "NiTriBasedGeom.h"

namespace nifDK::block_types {
   class NiTriStrips : public NiTriBasedGeom {
      public:
         static constexpr const char* const type_name = "NiTriStrips";
   };
}