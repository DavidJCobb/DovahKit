#pragma once
#include "./enums/button_press_type.h"
#include "./enums/control_type.h"
#include "./chrono.h"

namespace dovahkit::subsystems::worldinput {
   struct button_state;
   namespace inputs {
      struct bound_input;
   }

   struct input_result {
      control_type type = control_type::none;
      struct {
         timestamp_t       down_when  = zero_timestamp;
         button_press_type press_type = button_press_type::tap;
         bool              changed    = false; // whether the button was first pressed or was released on this frame; only relevant for "while down" button binds
      } button;
      float x = 0;
      float y = 0;

      bool active() const;
      bool is_button_release() const;

      static input_result for_button_input(timestamp_t now, const inputs::bound_input& bind, const button_state&);
      static input_result for_button_release();
   };
}