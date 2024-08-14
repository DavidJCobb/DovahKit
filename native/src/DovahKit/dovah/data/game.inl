#pragma once
#include "./game.h"

namespace dovah {
   constexpr bool game_supports_light_plugins(game g) {
      switch (g) {
         case game::skyrim_special:
            return true;
      }
      return false;
   }
}