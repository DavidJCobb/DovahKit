#pragma once
#include <cstdint>
#include <vector>
#include "../block.h"

namespace nifDK::block_types {
   class NiObject : public block {
      public:
         static constexpr const char* const type_name = "NiObject";
   };
}