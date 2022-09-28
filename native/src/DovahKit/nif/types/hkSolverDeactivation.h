#pragma once
#include <cstdint>

namespace nifDK {
   enum class hkSolverDeactivation : uint8_t {
      invalid,
      off,
      low,
      medium,
      high,
      maximum,
   };
}