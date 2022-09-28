#pragma once
#include <glm/glm.hpp>

namespace nifDK {
   class file_reader;

   struct StiffSpringDescriptor {
      glm::fvec4 pivot_a;
      glm::fvec4 pivot_b;
      float length;

      void read(file_reader&);
   };
}