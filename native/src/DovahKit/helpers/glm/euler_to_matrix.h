/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma once
#include <glm/glm.hpp>

namespace cobb::glm {
   enum class handedness {
      right,
      left,
   };

   template<handedness Handedness>
   ::glm::mat4 euler_intrinsic_xyz_to_mat(::glm::vec3 angles) {
      constexpr const float handedness_flip = (Handedness == handedness::right) ? -1.0 : 1.0;

      auto cx = cos(angles.x);
      auto cy = cos(angles.y);
      auto cz = cos(angles.z);
      auto sx = sin(angles.x * handedness_flip);
      auto sy = sin(angles.y * handedness_flip);
      auto sz = sin(angles.z * handedness_flip);

      ::glm::mat4 out;
      out[0][0] = cy*cz;
      out[0][1] = sx*cz*sy - cx*sz;
      out[0][2] = cx*sy*cz + sx*sz;
      out[0][3] = 0;
      out[1][0] = cy*sz;
      out[1][1] = sx*sy*sz + cx*cz;
      out[1][2] = cx*sy*sz - sx*cz;
      out[1][3] = 0;
      out[2][0] = -sy;
      out[2][1] = sx*cy;
      out[2][2] = cx*cy;
      out[2][3] = 0;
      out[3][0] = 0;
      out[3][1] = 0;
      out[3][2] = 0;
      out[3][3] = 1;
      return out;
   }

   template<handedness Handedness>
   ::glm::mat4 euler_intrinsic_zyx_to_mat(::glm::vec3 angles) {
      constexpr const float handedness_flip = (Handedness == handedness::right) ? -1.0 : 1.0;

      auto cx = cos(angles.x);
      auto cy = cos(angles.y);
      auto cz = cos(angles.z);
      auto sx = sin(angles.x * handedness_flip);
      auto sy = sin(angles.y * handedness_flip);
      auto sz = sin(angles.z * handedness_flip);

      ::glm::mat4 out;
      out[0][0] = cy*cz;
      out[0][1] = -cy*sz;
      out[0][2] = sy;
      out[0][3] = 0;
      out[1][0] = cx*sz + sx*sy*cz;
      out[1][1] = cx*cz - sx*sy*sz;
      out[1][2] = -sx*cy;
      out[1][3] = 0;
      out[2][0] = sx*sz - cx*sy*cz;
      out[2][1] = sx*cz + cx*sy*sz;
      out[2][2] = cx*cy;
      out[2][3] = 0;
      out[3][0] = 0;
      out[3][1] = 0;
      out[3][2] = 0;
      out[3][3] = 1;
      return out;
   }

   template<handedness Handedness>
   ::glm::mat4 euler_extrinsic_xyz_to_mat(::glm::vec3 angles) {
      return euler_intrinsic_zyx_to_mat<Handedness>(angles);
   }

   template<handedness Handedness>
   ::glm::mat4 euler_extrinsic_zyx_to_mat(::glm::vec3 angles) {
      return euler_intrinsic_xyz_to_mat<Handedness>(angles);
   }
}