#pragma once

namespace dovah {
   enum class game_change_failure_reason {
      // The load order contains too many dependencies, and can't be converted to a game 
      // that supports light plug-ins: it would overflow into the light plug-in slot.
      load_order_would_overflow_into_lights,

      // The load order contains light plug-ins, so the current file can't be converted 
      // to a game that doesn't support light plug-ins.
      load_order_contains_light_files,
   };
}