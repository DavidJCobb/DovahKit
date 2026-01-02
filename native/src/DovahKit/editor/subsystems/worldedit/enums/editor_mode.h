#pragma once

namespace dovahkit::subsystems::worldedit {
   enum class editor_mode {
      objects,
      navmesh,
      terrain,

      picking_ref, // i.e. "Select in Render Window" buttons
   };

   // REMINDER: When adding values, update not just the serialization options below, but 
   // also the bitcount used for the appropriate member on `condition_set`.
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldedit::editor_mode> {
   using value_type = dovahkit::subsystems::worldedit::editor_mode;

   static constexpr const size_t bitcount = 3;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ objects, navmesh, terrain, picking_ref };
   }();
};