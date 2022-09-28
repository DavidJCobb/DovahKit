#pragma once
#include "bhkTransformShape.h"

namespace nifDK::block_types {
   class bhkConvexTransformShape : public bhkTransformShape {
      public:
         static constexpr const char* const type_name = "bhkConvexTransformShape";
   };
}