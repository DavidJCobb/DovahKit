#pragma once

namespace dovah {
   enum class game {
      skyrim_classic,
      skyrim_special,
   };

   constexpr bool game_supports_light_plugins(game);
}

#include "./game.inl"