#include "LimitedHingeDescriptor.h"
#include "../reader.h"

namespace nifDK {
   void LimitedHingeDescriptor::read(file_reader& reader) {
      bool is_old_havok = reader.user_version<2>() <= 16;

      if (is_old_havok) {
         reader.read(this->transform_a[3]);
         reader.read(this->transform_a[0]);
         reader.read(this->transform_a[1]);
         reader.read(this->transform_a[2]);
         reader.read(this->transform_b[3]);
         reader.read(this->transform_b[0]);
         reader.read(this->transform_b[2]);
         this->transform_b[1] = glm::fvec4(glm::cross((glm::vec3)this->transform_b[2], (glm::vec3)this->transform_b[0]), 0.0);
      } else {
         reader.read(this->transform_a);
         reader.read(this->transform_b);
      }
      reader.read(this->angle.minimum);
      reader.read(this->angle.maximum);
      reader.read(this->max_friction);
      reader.read(this->motor);
   }
}