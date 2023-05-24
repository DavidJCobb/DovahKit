#pragma once

namespace dovahkit::subsystems::worldedit {
   enum class reference_frame {
      current = -1,
      //
      local  = 0,
      world  = 1,
      camera = 2,
   };
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldedit::reference_frame> {
   using value_type = dovahkit::subsystems::worldedit::reference_frame;

   static constexpr const size_t bitcount = 3;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ current, local, world, camera };
   }();
};