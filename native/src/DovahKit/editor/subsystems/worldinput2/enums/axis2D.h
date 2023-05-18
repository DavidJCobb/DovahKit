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