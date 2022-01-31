#pragma once
#include <glm/glm.hpp>
#include "NiMatrix33.h"

namespace nifDK {
   struct NiTransform {
      glm::vec3  position;
      NiMatrix33 rotation;
      float      scale = 1.0;

      inline glm::mat4 to_matrix() const {
         return glm::scale(
            glm::translate(
               glm::mat4(this->rotation),
               this->position
            ),
            glm::vec3{ this->scale, this->scale, this->scale }
         );
      }
   };
}