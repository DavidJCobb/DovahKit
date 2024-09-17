#pragma once
#include <cstdint>

namespace dovah::dialogue {
   enum class emotion : uint32_t {
      neutral  = 0,
      anger    = 1,
      disgust  = 2,
      fear     = 3,
      sad      = 4,
      happy    = 5,
      surprise = 6,
      puzzled  = 7,
   };
}