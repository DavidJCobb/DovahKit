#pragma once
#include "bhkShape.h"

namespace nifDK::block_types {
   class bhkShapeCollection : public bhkShape {
      public:
         static constexpr const char* const type_name = "bhkShapeCollection";
   };
}