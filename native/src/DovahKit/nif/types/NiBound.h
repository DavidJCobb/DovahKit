#pragma once
#include <glm/glm.hpp>

namespace nifDK {
   struct NiBound {
      glm::fvec3 center = { 0, 0, 0 };
      float      radius = 0;
   };
}