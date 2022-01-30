#pragma once
#include <glm/glm.hpp>
#include "NiMatrix33.h"

namespace nifDK {
   struct NiTransform {
      glm::vec3  position;
      NiMatrix33 rotation;
      float      scale = 1.0;
   };
}