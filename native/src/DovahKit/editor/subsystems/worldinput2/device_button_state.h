#pragma once
#include <cstdint>
#include <type_traits>
#include "./enums/button_press_type.h"
#include "./chrono.h"

namespace dovahkit::subsystems::worldinput2 {
   struct device_button_state {
      struct flag {
         flag() = delete;
         enum type : uint8_t {
            went_down_on_this_frame = 0x01,
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;

      // -----

      timestamp_t down_when = zero_timestamp; // when the key went down, or a zero timestamp
      flags_t     flags     = 0;

      button_press_type release_type = button_press_type::none;
      // If the button was released on this frame, this value indicates how long it was held 
      // down.  This is not necessarily the  press type per se.  Rather, consider what would 
      // happen if the user has "Press X", "Long Press X", and "Hold X" binds:  three binds; 
      // same key;  distinguished by press type.  In that situation, we would have to decide 
      // which bind to activate based on how long the user held the button. That's what this 
      // value helps to measure.
      //
      // If the button was not released on this frame, then this value is none.

      constexpr bool is_down() const noexcept {
         return this->down_when != zero_timestamp;
      }
      constexpr bool was_pressed_this_frame() const noexcept {
         return (this->flags & flag::went_down_on_this_frame) != 0;
      }
      constexpr bool was_released_this_frame() const noexcept {
         return !this->is_down() && this->release_type != button_press_type::none;
      }
   };
}