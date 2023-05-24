#pragma once

namespace dovahkit::subsystems::worldinput2 {
   // A physical input control that can produce two scalar values, e.g. the movement of a joystick or the mouse.
   enum class vector_input_control {
      none,
      mouse_move,
      xinput_ls,
      xinput_rs,
   };
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldinput2::vector_input_control> {
   using value_type = dovahkit::subsystems::worldinput2::vector_input_control;

   static constexpr const size_t bitcount = 2;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ none, mouse_move, xinput_ls, xinput_rs };
   }();
};