#pragma once
#include <glm/glm.hpp>

namespace nifDK {
   class file_reader;

   struct BallAndSocketDescriptor {
      glm::fvec4 pivot_a;
      glm::fvec4 pivot_b;

      void read(file_reader&);
   };
}