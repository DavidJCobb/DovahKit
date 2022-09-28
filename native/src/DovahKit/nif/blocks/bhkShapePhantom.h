#pragma once
#include "bhkPhantom.h"

namespace nifDK::block_types {
   class bhkShapePhantom : public bhkPhantom {
      public:
         static constexpr const char* const type_name = "bhkShapePhantom";
   };
}