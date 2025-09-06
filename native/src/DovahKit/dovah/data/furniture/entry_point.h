#pragma once
#include "helpers/enum_flags.h"

namespace dovah::furniture {
   enum class entry_point {
      front,
      back,
      right,
      left,
      up,
   };

   using entry_point_flags = cobb::enum_flags<entry_point, 32>;
}