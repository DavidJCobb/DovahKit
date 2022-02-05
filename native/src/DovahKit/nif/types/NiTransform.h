#pragma once
#include <glm/glm.hpp>
#include "NiMatrix33.h"

namespace nifDK {
   class file_reader;

   struct NiTransform {
      glm::vec3  position;
      NiMatrix33 rotation;
      float      scale = 1.0;

      void read(file_reader&);
      void unchecked_read(file_reader&);

      glm::mat4 to_matrix() const;
   };
}