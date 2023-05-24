#pragma once

namespace dovahkit::subsystems::worldedit {
   enum class editor_mode {
      objects,
      navmesh,
      terrain,
   };
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldedit::editor_mode> {
   using value_type = dovahkit::subsystems::worldedit::editor_mode;

   static constexpr const size_t bitcount = 2;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ objects, navmesh, terrain };
   }();
};