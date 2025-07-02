#pragma once
#include <cstdint>

namespace dovah::packages {
   enum class preferred_movement_speed : uint8_t {
      walk,
      jog,
      run,
      fast_walk,
   };
}