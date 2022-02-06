#include "NiTransform.h"
#include "../reader.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "NiMatrix33.h"

namespace nifDK {
   void NiTransform::read(file_reader& reader) {
      reader.require_size(sizeof(float) * (3 + 9 + 1));
      reader.unchecked_read(*this);
   }
   void NiTransform::unchecked_read(file_reader& reader) {
      reader.unchecked_read(this->position);
      reader.unchecked_read(this->rotation);
      reader.unchecked_read(this->scale);
   }

   glm::mat4 NiTransform::to_matrix() const {
      glm::mat4 out;
      for (int c = 0; c < 3; ++c)
         for (int r = 0; r < 3; ++r)
            out[c][r] = this->rotation[c][r] * this->scale;
      out[3] = glm::vec4(this->position, 1.0);
      return out;
      /*return glm::scale(
         glm::translate(
            glm::mat4(this->rotation),
            this->position
         ),
         glm::vec3{ this->scale, this->scale, this->scale }
      );*/
   }
}