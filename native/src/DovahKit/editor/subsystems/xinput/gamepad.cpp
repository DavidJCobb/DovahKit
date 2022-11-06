#include "gamepad.h"
#include "./config/trigger_threshold_for_button.h"

namespace dovahkit::subsystems::xinput {
   bool gamepad::is_button_down(button b) const noexcept {
      if (b < button::pseudo_buttons_start)
         return (this->buttons & (uint16_t)b) != 0;
      switch (b) {
         case button::trigger_left:
            return this->lt >= config::trigger_threshold_for_button;
         case button::trigger_right:
            return this->rt >= config::trigger_threshold_for_button;
      }
      return false;
   }
}