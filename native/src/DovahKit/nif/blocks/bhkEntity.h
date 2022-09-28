#pragma once
#include "bhkWorldObject.h"

namespace nifDK::block_types {
   class bhkEntity : public bhkWorldObject {
      public:
         static constexpr const char* const type_name = "bhkEntity";
   };
}