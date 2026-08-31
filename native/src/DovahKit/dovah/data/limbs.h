#pragma once
#include <cstdint>

namespace dovah {
   enum class limb : uint8_t {
      torso    = 0,
      head     = 1,
      eye      = 2,
      look_at  = 3,
      fly_grab = 4,
      saddle   = 5,
   };

   constexpr const size_t limbs_count = (size_t)limb::saddle + 1;
}