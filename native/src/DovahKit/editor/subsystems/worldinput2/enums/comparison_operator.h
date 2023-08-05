#pragma once

namespace dovahkit::subsystems::worldinput2 {
   enum class comparison_operator {
      equal,
      less,
      greater,
      not_equal,
      less_or_equal,
      greater_or_equal,
   };
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldinput2::comparison_operator> {
   using value_type = dovahkit::subsystems::worldinput2::comparison_operator;

   static constexpr const size_t bitcount = 3;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ equal, less, greater, not_equal, less_or_equal, greater_or_equal };
   }();
};