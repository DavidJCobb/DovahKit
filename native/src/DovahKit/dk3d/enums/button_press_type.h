#pragma once

namespace DK3D {
   enum class button_press_type {
      tap,        // run the tool when the button is released, if it was held only briefly
      hold,       // run the tool when the button is released, if it was held for longer than briefly
      while_down, // run the tool continuously while the button is held down, and then inform it when the button is released
   };
}