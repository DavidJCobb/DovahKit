#pragma once
#include <optional>
#include "../game.h"

namespace dovah {
   // Returns the minimum file header version needed for TES files in 
   // this game to cannibalize the hardcoded form ID range.
   constexpr std::optional<float> game_can_cannibalize_hardcoded_form_id_space(game g) {
      if (g == game::skyrim_special) {
         return 1.71;
      }
      return {};
   }
}