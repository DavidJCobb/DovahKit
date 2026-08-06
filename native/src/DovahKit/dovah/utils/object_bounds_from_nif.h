#pragma once
#include <limits>
#include <glm/glm.hpp>
namespace nifDK {
   class block;
}

namespace dovah::utils {
   struct object_bounds_float {
      glm::fvec3 min = {
         std::numeric_limits<float>::max(),
         std::numeric_limits<float>::max(),
         std::numeric_limits<float>::max(),
      };
      glm::fvec3 max = {
         std::numeric_limits<float>::lowest(),
         std::numeric_limits<float>::lowest(),
         std::numeric_limits<float>::lowest(),
      };
      
      constexpr bool is_undefined() const noexcept {
         return this->max.x < this->min.x;
      }
   };

   extern object_bounds_float object_bounds_from_nif(const nifDK::block&);
}