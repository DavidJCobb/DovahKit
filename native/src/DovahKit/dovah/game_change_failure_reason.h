#pragma once

namespace dovah {
   enum class game_change_failure_reason {
      // The load order contains too many dependencies, and can't be converted to a game 
      // that supports light plug-ins: it would overflow into the light plug-in slot.
      load_order_would_overflow_into_lights,

      // The load order contains light plug-ins, so the current file can't be converted 
      // to a game that doesn't support light plug-ins.
      load_order_contains_light_files,

      // The active file defines new forms in the range [xx000001, xx0007FF], and the 
      // target game doesn't support any version of the file format that allows this (all 
      // such IDs are treated as overriding hardcoded forms).
      active_file_cannibalizes_the_hardcoded_form_id_range,
   };
}