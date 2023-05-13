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