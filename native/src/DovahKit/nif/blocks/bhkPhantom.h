#pragma once
#include "bhkWorldObject.h"

namespace nifDK::block_types {
   class bhkPhantom : public bhkWorldObject {
      public:
         static constexpr const char* const type_name = "bhkPhantom";
   };
}