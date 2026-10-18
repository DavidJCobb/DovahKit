#pragma once
#include <cmath>
#include <limits>

namespace dovah::loaded_forms::structs {
   // Bounding box for a worldspace, measured in world units. Bounds are of the 
   // form [min, max), i.e. the max should be *past* the highest coords for any 
   // cell in the worldspace.
   struct world_bounds {
      struct {
         float x = std::numeric_limits<float>::max();
         float y = std::numeric_limits<float>::max();
      } min; // NAM0
      struct {
         float x = std::numeric_limits<float>::lowest();
         float y = std::numeric_limits<float>::lowest();
      } max; // NAM9

      constexpr bool valid() const noexcept {
         constexpr const auto maximum = std::numeric_limits<float>::max();
         constexpr const auto minimum = std::numeric_limits<float>::lowest();
         if (this->min.x == maximum || this->min.y == maximum)
            return false;
         if (this->max.x == minimum || this->max.y == minimum)
            return false;
         if (this->min.x > this->max.x || this->min.y > this->max.y)
            return false;
         if (!std::isfinite(this->min.x) || !std::isfinite(this->min.y) || !std::isfinite(this->max.x) || !std::isfinite(this->max.y))
            return false;
         return true;
      }
   };
}