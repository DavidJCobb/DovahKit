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
#include "./glm_transform_from_beth.h"
#include "../../helpers/glm/euler_to_matrix.h"

namespace vulkanDK {
   extern glm::mat4 glm_transform_from_beth(const glm::vec3& pos, glm::vec3 rot, float scale) {
      glm::mat4 transform = cobb::glm::euler_intrinsic_xyz_to_mat<cobb::glm::handedness::left>(rot);
      if (scale != 1.0)
         transform *= scale;
      transform[3] = glm::vec4(pos, 1);
      return transform;
   }
   extern glm::mat4 glm_transform_from_beth(const cobb::vector3<float>& pos, cobb::vector3<float> vr, float scale) {
      glm::mat4 transform = cobb::glm::euler_intrinsic_xyz_to_mat<cobb::glm::handedness::left>(vr.to_struct<glm::vec3>());
      if (scale != 1.0)
         transform *= scale;
      transform[3] = glm::vec4(pos.x, pos.y, pos.z, 1);
      return transform;
   }
}