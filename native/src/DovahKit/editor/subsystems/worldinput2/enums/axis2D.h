#pragma once

namespace dovahkit::subsystems::worldinput2 {
   enum class axis2D {
      x,
      y,
   };
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldinput2::axis2D> {
   using value_type = dovahkit::subsystems::worldinput2::axis2D;

   static constexpr const size_t bitcount = 1;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ x, y };
   }();
};