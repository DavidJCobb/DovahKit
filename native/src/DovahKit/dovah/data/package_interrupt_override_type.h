#pragma once
#include <cstdint>

namespace dovah {
   enum class package_interrupt_override_type : uint8_t {
      none,
      spectator,
      observe_dead,
      guard_warn,
      combat,
   };
}
