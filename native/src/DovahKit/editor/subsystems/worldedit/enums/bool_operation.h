#pragma once

namespace dovahkit::subsystems::worldedit {
   enum class bool_operation {
      no_op,
      set_false,
      set_true,
      invert,
   };
}

#include "helpers/streams/bitcount_of_enum.h"
template<>
constexpr const size_t cobb::streams::bitcount_of_enum<dovahkit::subsystems::worldedit::bool_operation> = 
   cobb::streams::bitcount_of_enum_member<dovahkit::subsystems::worldedit::bool_operation::invert>;

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldedit::bool_operation> {
   using value_type = dovahkit::subsystems::worldedit::bool_operation;

   static constexpr const size_t bitcount = 2;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ no_op, set_false, set_true, invert };
   }();
};