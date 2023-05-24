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
      using enum value_type;
      return std::array{ no_op, add, remove, toggle, replace };
   }();
};