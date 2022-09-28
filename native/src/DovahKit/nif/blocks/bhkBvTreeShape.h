#pragma once
#include "bhkShape.h"

namespace nifDK::block_types {
   class bhkBvTreeShape : public bhkShape {
      public:
         static constexpr const char* const type_name = "bhkBvTreeShape";
   };
}