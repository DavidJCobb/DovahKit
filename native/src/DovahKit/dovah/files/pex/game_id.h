#pragma once
#include <cstdint>

namespace dovah::pex {
   enum class game_id : uint16_t {
      skyrim     = 0x0001,
      fallout_4  = 0x0002,
      fallout_76 = 0x0003,
      starfield  = 0x0004,
   };
}
