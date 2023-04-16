#pragma once
#include <vector>
#include "./inputs/button.h"
#include "./chrono.h"

namespace dovahkit::subsystems::worldinput2 {
   struct interruption_check {
      struct potentially_interrupting_button {
         inputs::button button;
         timestamp_t    down_at;
         //
         bool matched = false;
      };

      std::vector<potentially_interrupting_button> buttons;
      size_t start_at = 0;
   };
}