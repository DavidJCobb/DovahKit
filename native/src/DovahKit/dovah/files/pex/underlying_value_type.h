#pragma once
#include <cstdint>

namespace dovah::pex {
   enum class underlying_value_type : uint8_t {
      none    = 0,
      object  = 1,
      string  = 2,
      integer = 3,
      float32 = 4,
      boolean = 5,
   };
}
