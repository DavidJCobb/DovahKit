#pragma once

namespace dovahkit::subsystems::worldinput2 {
   enum class optional_yn {
      unspecified,
      no,
      yes,
   };
}

#include "helpers/streams/bitcount_of_enum.h"
template<>
constexpr const size_t cobb::streams::bitcount_of_enum<dovahkit::subsystems::worldinput2::optional_yn> =
   cobb::streams::bitcount_of_enum_member<dovahkit::subsystems::worldinput2::optional_yn::yes>;