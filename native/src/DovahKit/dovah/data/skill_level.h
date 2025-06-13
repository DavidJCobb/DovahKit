#pragma once
#include <cstdint>

namespace dovah {
   enum class skill_level : int32_t {
      novice = 0,
      apprentice,
      journeyman,
      expert,
      master
   };
}
