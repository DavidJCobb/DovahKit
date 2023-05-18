#pragma once

namespace dovahkit::subsystems::worldinput2 {
   enum class input_device_type {
      keyboard_mouse,
      xinput,
   };
}

#include "helpers/streams/bitcount_of_enum.h"
template<>
constexpr const size_t cobb::streams::bitcount_of_enum<dovahkit::subsystems::worldinput2::input_device_type> =
   cobb::streams::bitcount_of_enum_member<dovahkit::subsystems::worldinput2::input_device_type::xinput>;