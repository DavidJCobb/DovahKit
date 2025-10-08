#pragma once
#include "../game.h"

namespace dovah::game_feature_support {
   constexpr float max_file_version(game g) {
      switch (g) {
         case game::skyrim_classic:
            return 1.70F;
         case game::skyrim_special:
            return 1.71F;
      }
      return 0;
   }
}