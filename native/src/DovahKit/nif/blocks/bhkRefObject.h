#pragma once
#pragma once
#include "NiObject.h"

namespace nifDK::block_types {
   class bhkRefObject : public NiObject {
      public:
         static constexpr const char* const type_name = "bhkRefObject";
   };
}