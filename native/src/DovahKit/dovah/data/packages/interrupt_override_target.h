#pragma once
#include <cstdint>
#include "./interrupt_override_type.h"

namespace dovah::packages {
   enum class interrupt_override_target : uint32_t {
      threat_to_spectate,
      corpse_to_observe,
      ref_to_guard,
      trespasser,
      combat_target,
   };

   constexpr interrupt_override_type interrupt_override_for_target(interrupt_override_target t) {
      switch (t) {
         case interrupt_override_target::threat_to_spectate:
            return interrupt_override_type::spectator;
         case interrupt_override_target::corpse_to_observe:
            return interrupt_override_type::observe_dead;
         case interrupt_override_target::ref_to_guard:
            return interrupt_override_type::guard_warn;
         case interrupt_override_target::trespasser:
            return interrupt_override_type::guard_warn;
         case interrupt_override_target::combat_target:
            return interrupt_override_type::combat;
      }
      return interrupt_override_type::none;
   }
}