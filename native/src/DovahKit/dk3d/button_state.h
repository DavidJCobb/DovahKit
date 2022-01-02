#pragma once
#include "enums/button_release_type.h"
#include "chrono.h"

namespace DK3D {
   struct button_state {
      //
      // Represents key state change information.
      // 
      //  - The (is_down) bool indicates that the key is pressed down, and that it isn't being 
      //    intentionally ignored.
      // 
      //  - The (down_when) value indicates at what time the key went down. If the key is not 
      //    down, then it equals (zero_timestamp).
      // 
      //  - The (released) value indicates how the key was released, if the key is not down. 
      //    A key can be "tapped" or "held." If the key is down, or if the key was not released 
      //    on this specific frame, then (release) is "none."
      //
      bool        is_down   = false;
      timestamp_t down_when = zero_timestamp;
      button_release_type released  = button_release_type::none;
   };
}