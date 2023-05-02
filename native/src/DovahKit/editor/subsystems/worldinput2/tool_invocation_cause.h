#pragma once
#include "./enums/button_press_type.h"
#include "./chrono.h"

namespace dovahkit::subsystems::worldinput2 {
   struct tool_invocation_cause {
      public:
         struct {
            bool down_state_changed_this_frame = false;
            bool is_down = false;
            //
            timestamp_t       down_when  = zero_timestamp;           // used for merging tool results, when ordering is relevant for that tool
            button_press_type press_type = button_press_type::press; // Hold binds may require different tool-side behaviors
         } button;
         struct {
            float x = 0;
            float y = 0;
            bool  is_delta = false;
         } range;

      protected:
         bool _is_button = true;

      public:
         constexpr const bool is_button() const { return this->_is_button; }
   };
}