#pragma once

namespace dovahkit::subsystems::worldedit {
   enum class sign {
      positive,
      negative,
   };
}

#include "helpers/streams/bitcount_of_enum.h"
template<>
constexpr const size_t cobb::streams::bitcount_of_enum<dovahkit::subsystems::worldedit::sign> =
   cobb::streams::bitcount_of_enum_member<dovahkit::subsystems::worldedit::sign::negative>;

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldedit::sign> {
   using value_type = dovahkit::subsystems::worldedit::sign;

   static constexpr const size_t bitcount = 1;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ positive, negative };
   }();
};