#pragma once
#include <cstdint>

namespace dovah::loaded_forms::structs {
   template<float Min, float Max>
   struct weather_transition_threshold {
      public:
         static constexpr const float minimum = Min;
         static constexpr const float maximum = Max;
         static constexpr const float step    = 1 / (Max - Min);

      public:
         uint8_t raw = 0;

      public:
         constexpr weather_transition_threshold() {}
         constexpr weather_transition_threshold(float f) {
            this->set_float(f);
         }
         explicit constexpr weather_transition_threshold(uint8_t raw) : raw(raw) {}

         constexpr operator float() const noexcept {
            return (float)this->raw / 255;
         }
         constexpr void set_float(float f) {
            if (f <= minimum) {
               this->raw = 0;
            } else if (f >= maximum) {
               this->raw = 255;
            } else {
               this->raw = f * 255.0F;
            }
         }
   };

   using weather_transition_intro_threshold = weather_transition_threshold<0.000F, 0.999F>;
   using weather_transition_outro_threshold = weather_transition_threshold<0.001F, 1.000F>;

   struct weather_transition_threshold_pair {
      weather_transition_intro_threshold intro;
      weather_transition_outro_threshold outro;
   };
}