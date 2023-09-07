#pragma once

namespace dovahkit::subsystems::worldinput {
   // A physical input control that can produce scalar values rather than boolean ones.
   enum class range_input_control {
      none,
      mouse_move,
      xinput_ls,
      xinput_rs,
      xinput_lt,
      xinput_rt,
   };

   constexpr bool range_input_control_has_multiple_axes(range_input_control r) {
      switch (r) {
         using enum range_input_control;
         case mouse_move:
         case xinput_ls:
         case xinput_rs:
            return true;
      }
      return false;
   }
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldinput::range_input_control> {
   using value_type = dovahkit::subsystems::worldinput::range_input_control;

   static constexpr const size_t bitcount = 3;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ none, mouse_move, xinput_ls, xinput_rs, xinput_lt, xinput_rt };
   }();
};