#pragma once
#include <array>
#include "helpers/bitfield_array.h"
#include "../../enums/button_press_type.h"
#include "../../chrono.h"
#include "../../device_button_state.h"

namespace dovahkit::subsystems::worldinput2::devices::components {
   //
   // Stores `device_button_state` for an indexed set of buttons, as a struct-of-arrays. 
   // A device handler class should include an instance of this as a member, and should 
   // map device-specific button identifiers (e.g. XInput button enums; VK codes) into 
   // button indices.
   //
   template<size_t ButtonCount> struct buttoned_device {
      public:
         static constexpr const size_t button_count = ButtonCount;

      public:
         std::array<timestamp_t, ButtonCount> start = {};
         cobb::bitfield_array<device_button_state::flags_t, ButtonCount, 3> flags;
         cobb::bitfield_array<button_press_type, ButtonCount, 3> release_times;

      public:
         // Per-frame update handler; call per button.
         void update_button(timestamp_t now, size_t index, bool is_down);

         // Call if the device was disconnected on this frame.
         void handle_disconnected();

         void ignore_all_down();

         device_button_state get_button_state(size_t index) const;
         constexpr bool is_consumed(size_t index) const {
            return this->flags[index] & device_button_state::flag::consumed_on_a_previous_frame;
         }
   };
}

#include "./buttoned_device.inl"