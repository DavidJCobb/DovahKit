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