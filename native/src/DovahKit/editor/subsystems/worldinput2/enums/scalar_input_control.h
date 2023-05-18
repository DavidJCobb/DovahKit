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

#include "helpers/streams/bitcount_of_enum.h"
template<>
constexpr const size_t cobb::streams::bitcount_of_enum<dovahkit::subsystems::worldinput2::scalar_input_control> =
   cobb::streams::bitcount_of_enum_member<dovahkit::subsystems::worldinput2::scalar_input_control::xinput_rt>;