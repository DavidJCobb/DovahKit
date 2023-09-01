#pragma once
#include "./rgb.h"
#include <cmath>

namespace cobb::color {
   rgb_floats rgb_floats::sRGB_to_linear() const {
      rgb_floats out;
      for (size_t i = 0; i < 3; ++i) {
         auto  s = this->components[i];
         auto& l = out.components[i];
         if (s <= 0.04045) {
            if (s < 0)
               s = 0;
            l = s / 12.92;
         } else {
            if (s > 1)
               s = 1;
            l = std::pow((s + 0.055) / 1.055, 2.4);
         }
      }
      return out;
   }
   rgb_floats rgb_floats::linear_to_sRGB() const {
      rgb_floats out;
      for (size_t i = 0; i < 3; ++i) {
         auto  l = this->components[i];
         auto& s = out.components[i];
         if (l <= 0.0031308) {
            if (l < 0)
               l = 0;
            s = l * 12.92;
         } else {
            if (l > 1)
               l = 1;
            s = 1.055 * std::pow(l, 1 / 2.4) - 0.055;
         }
      }
      return out;
   }
}
