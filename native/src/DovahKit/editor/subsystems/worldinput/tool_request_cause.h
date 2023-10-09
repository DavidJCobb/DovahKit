#pragma once
#include "./enums/button_press_type.h"
#include "./enums/range_input_axes.h"
#include "./chrono.h"
#include "./raycast_result.h"

namespace dovahkit::subsystems::worldinput {
   struct tool_request_cause {
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
            range_input_axes axes = range_input_axes::all;
         } range;
         //
         bool has_button : 1 = false;
         bool has_range  : 1 = false;
         //
         std::optional<raycast_result> raycast;
   };
}