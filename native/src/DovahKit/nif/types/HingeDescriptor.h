#pragma once
#include <glm/glm.hpp>

namespace nifDK {
   class file_reader;

   struct HingeDescriptor {
      glm::fmat4x4 transform_a;
      glm::fmat4x4 transform_b;

      void read(file_reader&);
   };
}