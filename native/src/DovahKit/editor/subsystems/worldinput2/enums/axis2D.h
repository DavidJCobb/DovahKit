#pragma once

namespace dovahkit::subsystems::worldinput2 {
   enum class axis2D {
      x,
      y,
   };
}

#include "helpers/streams/bitcount_of_enum.h"
template<>
constexpr const size_t cobb::streams::bitcount_of_enum<dovahkit::subsystems::worldinput2::axis2D> =
   cobb::streams::bitcount_of_enum_member<dovahkit::subsystems::worldinput2::axis2D::y>;

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldinput2::axis2D> {
   using value_type = dovahkit::subsystems::worldinput2::axis2D;

   static constexpr const size_t bitcount = 1;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ x, y };
   }();
};