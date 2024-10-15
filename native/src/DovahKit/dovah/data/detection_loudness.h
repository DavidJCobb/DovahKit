#pragma once
#include <cstdint>

namespace dovah {
   enum class detection_loudness : uint32_t {
      loud,
      normal,
      silent,
      very_loud,
   };
}