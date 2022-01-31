#pragma once
#include "NiGeometry.h"

namespace nifDK::block_types {
   class NiTriBasedGeom : public NiGeometry {
      public:
         static constexpr const char* const type_name = "NiTriBasedGeom";
   };
}