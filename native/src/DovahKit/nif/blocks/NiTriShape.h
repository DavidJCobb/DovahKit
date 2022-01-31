#pragma once
#include "NiTriBasedGeom.h"

namespace nifDK::block_types {
   class NiTriShape : public NiTriBasedGeom {
      public:
         static constexpr const char* const type_name = "NiTriShape";
   };
}