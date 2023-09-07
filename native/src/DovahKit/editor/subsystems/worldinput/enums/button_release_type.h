#pragma once

namespace dovahkit::subsystems::worldinput {
   enum class button_release_type {
      tap,  // the button was released on this frame after being held briefly
      hold, // the button was released on this frame after being held for any amount of time longer than "briefly"
      none, // the button was not released on this frame
   };
}