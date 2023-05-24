#pragma once
#include "vulkan/enums/gizmo_mode.h"

namespace dovahkit::subsystems::worldedit {
   using gizmo_mode = vulkanDK::gizmo_mode;
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldedit::gizmo_mode> {
   using value_type = dovahkit::subsystems::worldedit::gizmo_mode;

   static constexpr const size_t bitcount = 2;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ none, translate, rotate, scale };
   }();
};