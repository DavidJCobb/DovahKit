#pragma once
#include <cstdint>
#include <limits>
#include "helpers/math.h"

namespace vulkanDK {
   class fps_tracker {
      public:
         enum class counting_mode {
            raw,     // display the FPS for each frame individually
            average, // take a rolling average of each frame's FPS
            count,   // count how many frames have rendered at one-second intervals
         };

         // Configuration:
         static constexpr counting_mode mode         = counting_mode::count;
         static constexpr size_t        max_digits   =  5; // max digits to display
         static constexpr uint8_t       display_base = 10; // display numbers in base-10
         
         using delta_type = double;
         using value_type = int32_t;
         static constexpr value_type max_value = std::numeric_limits<value_type>::max();
         static constexpr value_type max_visible_value = cobb::pow((size_t)display_base, max_digits) - 1;

      public:
         value_type value = 0;
         struct {
            delta_type total = 0.0; // rolling average per frame, or total measured frame time for per-second averages
            size_t     count = 0;
         } history;

         void next_delta(delta_type);

         value_type display_value() const;
   };
}