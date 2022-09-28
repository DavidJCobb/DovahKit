#pragma once
#include "bhkSerializable.h"

namespace nifDK::block_types {
   class bhkShape : public bhkSerializable {
      public:
         static constexpr const char* const type_name = "bhkShape";
   };
}