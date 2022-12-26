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
#include "glm_transform_from_beth.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace {
   constexpr bool glm_is_righthanded = true;
   constexpr bool skyrim_is_righthanded = false;
}

namespace vulkanDK {
   extern glm::vec3 glm_transform_rotation_from_beth(const glm::vec3& rot) {
      if constexpr (glm_is_righthanded == skyrim_is_righthanded)
         return rot;
      return rot * -1.0F;
   }
   extern glm::vec3 glm_transform_rotation_from_beth(const cobb::vector3<float>& rot) {
      if constexpr (glm_is_righthanded == skyrim_is_righthanded)
         return rot.to_struct<glm::vec3>();
      return glm::vec3{ -rot.x, -rot.y, -rot.z };
   }
   extern void glm_transform_rotation_from_beth_in_place(glm::vec3& rot) {
      if constexpr (glm_is_righthanded == skyrim_is_righthanded)
         return;
      rot *= -1.0F;
   }
   extern void glm_transform_rotation_from_beth_in_place(float& x, float& y, float& z) {
      if constexpr (glm_is_righthanded == skyrim_is_righthanded)
         return;
      x = -x;
      y = -y;
      z = -z;
   }

   extern glm::mat4 glm_transform_from_beth(const glm::vec3& pos, glm::vec3 rot, float scale) {
      glm_transform_rotation_from_beth_in_place(rot);
      //
      glm::mat4 transform = glm::eulerAngleXYZ(rot.x, rot.y, rot.z);
      if (scale != 1.0)
         transform *= scale;
      transform[3] = glm::vec4(pos, 1);
      return transform;
   }
   extern glm::mat4 glm_transform_from_beth(const cobb::vector3<float>& pos, cobb::vector3<float> vr, float scale) {
      auto rot = glm_transform_rotation_from_beth(vr);
      //
      glm::mat4 transform = glm::eulerAngleXYZ(rot.x, rot.y, rot.z);
      if (scale != 1.0)
         transform *= scale;
      transform[3] = glm::vec4(pos.x, pos.y, pos.z, 1);
      return transform;
   }
}