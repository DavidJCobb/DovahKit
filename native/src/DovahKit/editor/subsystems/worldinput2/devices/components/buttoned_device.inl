#pragma once
#include "./buttoned_device.h"
#include <limits>
#include "../../defaults.h"

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#define TEMPLATE_PARAMS template<size_t ButtonCount>
#define CLASS_NAME buttoned_device<ButtonCount>

namespace dovahkit::subsystems::worldinput2::devices::components {
   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::update_button(timestamp_t now, size_t index, bool is_down) {
      auto& start = this->start[index];
      this->flags[index] &= ~device_button_state::flag::down_state_changed_on_this_frame;

      bool changed_this_frame = false;
      if (is_down) {
         if (start == zero_timestamp) {
            //
            // Key was up, last we checked.
            //
            start = now;
            this->flags[index] |= device_button_state::flag::down_state_changed_on_this_frame;
            changed_this_frame = true;
         }
      } else {
         if (start != zero_timestamp) {
            auto down_duration = elapsed_time(start, now);

            start = zero_timestamp;
            this->flags[index] |= device_button_state::flag::down_state_changed_on_this_frame;
            changed_this_frame = true;
         }
      }

      auto& claims = this->claims[index];
      if (!is_down && !changed_this_frame) {
         claims.existing = {};
      } else {
         if (claims.pending.specificity >= claims.existing.specificity) {
            claims.existing = claims.pending;
         }
      }
      claims.pending = {};
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::handle_disconnected() {
      this->ignore_all_down();
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::ignore_all_down() {
      for (size_t i = 0; i < button_count; ++i) {
         if (this->start[i] != zero_timestamp) {
            this->claims[i].existing.specificity = std::numeric_limits<size_t>::max();
         }
      }
   }

   TEMPLATE_PARAMS
   constexpr device_button_state CLASS_NAME::get_button_state(size_t index) const {
      device_button_state out;
      out.down_when = this->start[index];
      out.flags     = this->flags[index];
      return out;
   }
}

#undef CLASS_NAME
#undef TEMPLATE_PARAMS
#pragma pop_macro("CLASS_NAME")
#pragma pop_macro("TEMPLATE_PARAMS")