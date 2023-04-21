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
            down_state_changed_on_this_frame = 0x01,
            is_down = 0x02,
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;
      //
      static constexpr const size_t flag_count = 2;

      // -----

      timestamp_t down_when = zero_timestamp; // when the key last went down
      flags_t     flags     = 0;

      constexpr bool is_down() const noexcept {
         return (this->flags & flag::is_down) != 0;
      }
      constexpr bool was_pressed_this_frame() const noexcept {
         return this->is_down() && (this->flags & flag::down_state_changed_on_this_frame) != 0;
      }
      constexpr bool was_released_this_frame() const noexcept {
         return !this->is_down() && (this->flags & flag::down_state_changed_on_this_frame) != 0;
      }
   };
}