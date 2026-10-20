#pragma once
#include "dovah/game.h"

namespace dovah::facts {
   constexpr float min_file_version(game g) {
      switch (g) {
         case game::skyrim_classic:
            return 0.94F;
         case game::skyrim_special:
            return 1.70F;
      }
      return 0;
   }
}