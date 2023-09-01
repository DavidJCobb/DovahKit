#pragma once
#include "./rgb.h"

namespace cobb::color {
   constexpr rgb_bytes::operator rgb_floats() const {
      rgb_floats out = { (float)r, (float)g, (float)b };
      out.r /= 255.0;
      out.g /= 255.0;
      out.b /= 255.0;
      return out;
   }

   constexpr void rgb_floats::clamp() {
      if (this->r < 0)
         this->r = 0;
      else if (this->r > 1)
         this->r = 1;

      if (this->g < 0)
         this->g = 0;
      else if (this->g > 1)
         this->g = 1;

      if (this->b < 0)
         this->b = 0;
      else if (this->b > 1)
         this->b = 1;
   }
   constexpr rgb_floats rgb_floats::clamped() const {
      auto out = *this;
      out.clamp();
      return out;
   }

   constexpr rgb_floats::operator rgb_bytes() const {
      auto src = this->clamped();
      src.r *= 255.0;
      src.g *= 255.0;
      src.b *= 255.0;
      return { (uint8_t)src.r, (uint8_t)src.g, (uint8_t)src.b };
   }
}
