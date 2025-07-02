#pragma once
#include <cstdint>

namespace dovah::packages {
   enum class interrupt_override_type : uint8_t {
      none,
      spectator,
      observe_dead,
      guard_warn,
      combat,
   };
}
