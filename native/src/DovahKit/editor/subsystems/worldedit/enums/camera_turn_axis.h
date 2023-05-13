#pragma once

namespace dovahkit::subsystems::worldedit {
   enum class camera_turn_axis {
      yaw,
      pitch,
   };
}

#include "helpers/streams/bitcount_of_enum.h"
template<>
constexpr const size_t cobb::streams::bitcount_of_enum<dovahkit::subsystems::worldedit::camera_turn_axis> =
   cobb::streams::bitcount_of_enum_member<dovahkit::subsystems::worldedit::camera_turn_axis::pitch>;