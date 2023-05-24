#pragma once
#include <cstdint>

namespace dovahkit::subsystems::worldedit {
   enum class axis3D : uint8_t {
      x,
      y,
      z,
   };
}

#include "helpers/streams/bitcount_of_enum.h"
template<>
constexpr const size_t cobb::streams::bitcount_of_enum<dovahkit::subsystems::worldedit::axis3D> =
   cobb::streams::bitcount_of_enum_member<dovahkit::subsystems::worldedit::axis3D::z>;

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldedit::axis3D> {
   using value_type = dovahkit::subsystems::worldedit::axis3D;

   static constexpr const size_t bitcount = 2;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ x, y, z };
   }();
};