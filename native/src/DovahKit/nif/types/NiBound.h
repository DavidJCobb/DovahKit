#pragma once
#include <glm/glm.hpp>

namespace nifDK {
   class file_reader;

   struct NiBound {
      glm::fvec3 center = { 0, 0, 0 };
      float      radius = 0;

      void read(file_reader&);
      void unchecked_read(file_reader&);
   };
}