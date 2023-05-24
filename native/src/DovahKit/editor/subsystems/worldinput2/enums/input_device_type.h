#pragma once

namespace dovahkit::subsystems::worldinput2 {
   enum class input_device_type {
      keyboard_mouse,
      xinput,
   };
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldinput2::input_device_type> {
   using value_type = dovahkit::subsystems::worldinput2::input_device_type;

   static constexpr const size_t bitcount = 1;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ keyboard_mouse, xinput };
   }();
};