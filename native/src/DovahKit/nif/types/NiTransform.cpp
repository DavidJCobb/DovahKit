#include "NiTransform.h"
#include "../reader.h"
#include "vulkan/helpers/glm_transform_from_beth.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include "NiMatrix33.h"

namespace {
   constexpr bool glm_is_righthanded = false;
   constexpr bool skyrim_is_righthanded = false;
}

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
      glm::mat4 out = glm::mat4(this->rotation);
      out = glm::transpose(out); // GLM is column-major; NiMatrix33 is row-major.
      /*
      // Consider:
      //
      //   return glm::scale(
      //      glm::translate(this->position),
      //      glm::vec3(this->scale)
      //   ) * glm::mat4(this->rotation);
      //
      // glm::scale just multiplies the first three columns of the first argument by the three 
      // scalars (one per axis) supplied in the second argument. NiTransform only does uniform 
      // scaling, so the scalars will all be equivalent; and the input matrix is promoted from 
      // a 3x3 matrix, so in practice the fourth column will be {0, 0, 0, 1} initially; we can 
      // just go ahead and multiply the whole matrix by the scalar.
      //
      // Next, we apply the translation. In practice,  this literally just replaces the fourth 
      // column with the translation promoted to a vec4 (fixing the multiplied [3][3] value in 
      // the process), while changing nothing else.
      //
      //*/
      out *= this->scale;
      out[3] = glm::vec4(this->position, 1);
      return out;
   }
}