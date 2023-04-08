#pragma once
#include <vector>
#include "./inputs/button.h"
#include "./chrono.h"

namespace dovahkit::subsystems::worldinput2 {
   struct interruption_check {
      struct potentially_interrupting_button {
         const inputs::button button;
         const timestamp_t    down_at;
         bool matched = false;
      };

      std::vector<potentially_interrupting_button> buttons;
      size_t start_at = 0;
      //
      bool   saw_separate_ordered_group = false;
   };
}