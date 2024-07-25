#pragma once

namespace dovah {
   // Most content in the game engine is male by default, with an "is female" flag.
   enum class sex {
      male   = 0,
      female = 1,
   };
   constexpr const size_t sex_count = 2;
}