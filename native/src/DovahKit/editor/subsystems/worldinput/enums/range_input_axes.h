#pragma once

namespace dovahkit::subsystems::worldinput {
   // A physical input control that can produce scalar values rather than boolean ones.
   enum class range_input_axes {
      all,
      x,
      y,
   };
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldinput::range_input_axes> {
   using value_type = dovahkit::subsystems::worldinput::range_input_axes;

   static constexpr const size_t bitcount = 2;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ all, x, y };
   }();
};