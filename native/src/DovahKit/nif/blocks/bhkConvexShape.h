#pragma once
#include "bhkSphereRepShape.h"

namespace nifDK::block_types {
   class bhkConvexShape : public bhkSphereRepShape {
      public:
         static constexpr const char* const type_name = "bhkConvexShape";
   };
}