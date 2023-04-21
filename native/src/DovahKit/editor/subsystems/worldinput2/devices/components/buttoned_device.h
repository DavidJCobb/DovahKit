#pragma once
#include <array>
#include "helpers/bitfield_array.h"
#include "../../chrono.h"
#include "../../device_button_claim.h"
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
         std::array<timestamp_t, ButtonCount> down_at = {};
         cobb::bitfield_array<device_button_state::flags_t, ButtonCount, device_button_state::flag_count> flags;
         std::array<device_button_claim_set, ButtonCount> claims = {};

      public:
         // Per-frame update handler; call per button.
         constexpr void update_button(timestamp_t now, size_t index, bool is_down);

         // Call if the device was disconnected on this frame.
         constexpr void handle_disconnected();

         constexpr void ignore_all_down();

         constexpr device_button_state get_button_state(size_t index) const;

         constexpr bool is_consumed(size_t index) const {
            return this->flags[index] & device_button_state::flag::consumed_on_a_previous_frame;
         }
   };
}

#include "./buttoned_device.inl"