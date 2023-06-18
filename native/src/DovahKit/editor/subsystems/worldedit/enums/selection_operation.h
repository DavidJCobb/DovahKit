#pragma once

namespace dovahkit::subsystems::worldedit {
   enum class selection_operation {
      no_op,
      add,     // select object
      remove,  // deselect object
      toggle,  // toggle selection state of object
      replace, // replace entire selection
   };
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldedit::selection_operation> {
   using value_type = dovahkit::subsystems::worldedit::selection_operation;

   static constexpr const size_t bitcount = 3;
   static constexpr const auto   valid_values = []() {
      return std::array{ value_type::no_op, value_type::add, value_type::remove, value_type::toggle, value_type::replace };
   }();
};