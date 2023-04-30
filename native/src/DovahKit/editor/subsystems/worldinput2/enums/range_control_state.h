#pragma once

namespace dovahkit::subsystems::worldinput2 {
   enum class range_control_state {
      zeroed, // The control is at a zero position (if not delta), or is not being moved (if delta).
      stale,  // The control is a delta control, and movement has not timed out yet.
      active, // The control is at a non-zero position (if not delta) or is being moved (if delta).

      unavailable, // The control does not exist on this input device.
   };
}