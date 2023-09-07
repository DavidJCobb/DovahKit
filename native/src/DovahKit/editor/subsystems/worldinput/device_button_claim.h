#pragma once
#include "./chrono.h"

namespace dovahkit::subsystems::worldinput {
   struct device_button_claim {
      timestamp_t when        = zero_timestamp;
      size_t      specificity = 0;

      constexpr void attempt_new_claim(timestamp_t now, size_t specificity) {
         if (specificity < this->specificity)
            return;
         this->when        = now;
         this->specificity = specificity;
      }
   };

   struct device_button_claim_set {
      device_button_claim existing;
      device_button_claim pending;
   };
}