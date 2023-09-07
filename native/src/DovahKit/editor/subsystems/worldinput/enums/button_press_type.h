#pragma once
#include <cstdint>

namespace dovahkit::subsystems::worldinput {
   enum class button_press_type : int8_t {
      none       = -1,
      press      =  0, // run the tool when the button is released, if it was held only briefly
      long_press,      // run the tool when the button is released, if it was held down for longer than briefly
      hold,            // run the tool continuously while the button is held down, and then inform it when the button is released
   };
}