#pragma once

namespace dovahkit::subsystems::worldedit {
   enum class axis3D {
      x,
      y,
      z,
   };
}

#include "helpers/streams/bitcount_of_enum.h"
template<>
constexpr const size_t cobb::streams::bitcount_of_enum<dovahkit::subsystems::worldedit::axis3D> =
   cobb::streams::bitcount_of_enum_member<dovahkit::subsystems::worldedit::axis3D::z>;