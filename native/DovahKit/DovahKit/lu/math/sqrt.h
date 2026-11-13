#pragma once
#include <cmath>
#include <limits>

namespace lu {
   constexpr double sqrt(double v) noexcept {
      if (std::is_constant_evaluated()) {
         if (v < 0 || v >= std::numeric_limits<double>::infinity())
            return std::numeric_limits<double>::quiet_NaN();
         //
         // Use the Newton-Raphson method:
         //
         double current  = v;
         double previous = 0;
         while (current != previous) {
            previous = current;
            current  = 0.5 * (current + (v / current));
         }
         return current;
      } else {
         return std::sqrt(v);
      }
   }
}