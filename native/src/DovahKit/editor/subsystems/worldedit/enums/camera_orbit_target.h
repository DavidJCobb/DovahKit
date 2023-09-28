#pragma once

namespace dovahkit::subsystems::worldedit {
   enum class camera_orbit_target {
      primary_selection,
      selection_centroid,
   };
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldedit::camera_orbit_target> {
   using value_type = dovahkit::subsystems::worldedit::camera_orbit_target;

   static constexpr const size_t bitcount = 1;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ primary_selection, selection_centroid };
   }();
};