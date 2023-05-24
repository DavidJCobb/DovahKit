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

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldedit::camera_turn_axis> {
   using value_type = dovahkit::subsystems::worldedit::camera_turn_axis;

   static constexpr const size_t bitcount = 1;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ yaw, pitch };
   }();
};