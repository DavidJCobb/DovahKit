#pragma once

namespace dovahkit::subsystems::worldinput2 {
   // A physical input control that can produce one scalar value, e.g. a joystick trigger or a single axis on a joystick or mouse movement.
   enum class scalar_input_control {
      none,
      mouse_move,
      xinput_ls,
      xinput_rs,
      xinput_lt,
      xinput_rt,
   };
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldinput2::scalar_input_control> {
   using value_type = dovahkit::subsystems::worldinput2::scalar_input_control;

   static constexpr const size_t bitcount = 3;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ none, mouse_move, xinput_ls, xinput_rs, xinput_lt, xinput_rt };
   }();
};