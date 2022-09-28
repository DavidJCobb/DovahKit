#include "RagdollDescriptor.h"
#include "../reader.h"

namespace nifDK {
   void RagdollDescriptor::read(file_reader& reader) {
      bool is_old_havok = reader.user_version<2>() <= 16;

      if (is_old_havok) {
         reader.read(this->transform_a[3]);
         reader.read(this->transform_a[1]);
         reader.read(this->transform_a[0]);
         reader.read(this->transform_b[3]);
         reader.read(this->transform_b[1]);
         reader.read(this->transform_b[0]);
         this->transform_a[2] = glm::fvec4(glm::cross((glm::vec3)this->transform_a[0], (glm::vec3)this->transform_a[1]), 0.0); // blind random guess as to which axes to use
         this->transform_b[2] = glm::fvec4(glm::cross((glm::vec3)this->transform_b[0], (glm::vec3)this->transform_b[1]), 0.0); // blind random guess as to which axes to use
      } else {
         reader.read(this->transform_a);
         reader.read(this->transform_b);
      }
      reader.read(this->cone_max_angle);
      reader.read(this->plane_angle.minimum);
      reader.read(this->plane_angle.maximum);
      reader.read(this->twist_angle.minimum);
      reader.read(this->twist_angle.maximum);
      reader.read(this->max_friction);
      reader.read(this->motor);
   }
}