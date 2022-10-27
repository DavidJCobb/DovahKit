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
   constexpr bool glm_is_righthanded = true;
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
      if constexpr (glm_is_righthanded == skyrim_is_righthanded) {
         //
         // Skyrim uses lefthanded XYZ Euler, where Z+ is upward, Y+ is forward, and X+ is right. 
         // GLM, on the other hand, is righthanded by default. This means that we need to convert 
         // our handedness.
         // 
         // Sadly, there's no quick shortcut. Given separate matrices for X, Y, and Z, you can 
         // swap handedness by taking the transpose of each... but that doesn't work once they've 
         // been multiplied together. We have to extract our Euler values, negate them all, and 
         // then combine them back into a new matrix.
         //
         {
            //
            // For an explanation, see the `/cobb/rotation.cpp` file.
            //
            constexpr float EPSILON = 0.000001;
            //
            float x;
            float y = asin(-out[2][0]); // col 2, row 0 == sin(y)
            float z;
            //
            double u; // cos_
            double v; // sin_
            //
            float cosY = cos(y);
            if (fabs(cosY) > EPSILON) {
               u = out[2][2] / cosY; // u == cosX
               v = out[2][1] / cosY; // v == sinX
               x = atan2(v, u);
               //
               u = out[0][0] / cosY; // u == cosZ
               v = out[1][0] / cosY; // v == sinZ
               z = atan2(v, u);
            } else {
               z = 0;
               u = out[1][1]; // -sinX*sinY*sinZ + cosX*cosZ == -sinX*sinY*0 + cosX*1 == cosX
               v = -out[0][1]; //  cosX*sinY*sinZ + sinX*cosZ ==  cosX*sinY*0 + sinX*1 == sinX
               x = atan2(v, u);
            }
            //
            // Now that we've extracted the coordinates, we need to swap the handedness and then 
            // pass them into a GLM function to build a left-handed matrix out of the now left-
            // handed values.
            //
            x = -x;
            y = -y;
            z = -z;
            out = glm::eulerAngleXYZ(x, y, z);
         }
      }
      /*
      // Consider:
      //
      //   return glm::scale(
      //      glm::translate(this->position),
      //      glm::vec3(this->scale)
      //   ) * glm::mat4(this->rotation);
      //
      // The above code would work if NIF angles were righthanded.
      //
      // glm::scale just multiplies the first three columns of the first argument by the three 
      // scalars (one per axis) supplied in the second argument. NiTransform only does uniform 
      // scaling, so the scalars will all be equivalent; and the input matrix is promoted from 
      // a 3x3 matrix, so in practice the fourth column will be {0, 0, 0, 1} initially; we can 
      // just go ahead and multiply the whole matrix by the scalar.
      //
      // Next, we apply the translation. In practice,  this literally just replaces the fourth 
      // column with the translation promoted to a vec4, while changing nothing else.
      //
      //*/
      out *= this->scale;
      out[3] = glm::vec4(this->position, 1);
      return out;
   }
}