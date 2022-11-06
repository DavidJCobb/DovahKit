#pragma once
#include "helpers/function_pointer.h"
#include "./chrono.h"

namespace dovahkit::subsystems::xinput {
   using vibration_easing_function = cobb::function_pointer<float(float percent, float magnitude)>;

   extern constexpr float default_vibration_easing_function(float p, float m) {
      return m * p;
   }

   struct vibration {
      timestamp_t start;
      float       duration = 0.0;
      float       magnitude = 0.0;

      vibration_easing_function easing = &default_vibration_easing_function;
   };
}