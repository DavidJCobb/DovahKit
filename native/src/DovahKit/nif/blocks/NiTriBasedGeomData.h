#pragma once
#include "NiGeometryData.h"

namespace nifDK::block_types {
   class NiTriBasedGeomData : public NiGeometryData {
      public:
         static constexpr const char* const type_name = "NiTriBasedGeomData";
   };
}