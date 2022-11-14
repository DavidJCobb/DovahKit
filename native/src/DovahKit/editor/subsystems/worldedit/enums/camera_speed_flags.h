#pragma once
#include "helpers/enum_flags.h"

namespace dovahkit::subsystems::worldedit {
   enum class camera_speed_flag {
      boost,
      precision,
   };

   using camera_speed_flags = cobb::enum_flags<camera_speed_flag, 2>;
}