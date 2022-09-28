#pragma once
#include "bhkRefObject.h"

namespace nifDK::block_types {
   class bhkSerializable : public bhkRefObject {
      public:
         static constexpr const char* const type_name = "bhkSerializable";
   };
}