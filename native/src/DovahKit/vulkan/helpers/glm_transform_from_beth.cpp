#include "glm_transform_from_beth.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace {
   template<typename T> void _swap_handedness(T& rot) {
      /*//
      if (fabs(rot.x) > fabs(rot.z))
         rot.x = -rot.x;
      else if (fabs(rot.y) > fabs(rot.z))
         rot.y = -rot.y;
      else
         rot.z = -rot.z;
      //*/
      std::swap(rot.x, rot.y);
   }
}

namespace vulkanDK {
   extern glm::mat4 glm_transform_from_beth(const glm::vec3& pos, glm::vec3 rot, float scale) {
      //
      // We'll need a handedness flip from righthanded (Skyrim) to lefthanded (OpenGL/GLM).
      //
      _swap_handedness(rot);
      //
      glm::mat4 transform = glm::eulerAngleXYZ(rot.x, rot.y, rot.z);
      if (scale != 1.0)
         transform *= scale;
      transform[3] = glm::vec4(pos, 1);
      return transform;
   }
   extern glm::mat4 glm_transform_from_beth(const cobb::vector3<float>& pos, cobb::vector3<float> rot, float scale) {
      //
      // We'll need a handedness flip from righthanded (Skyrim) to lefthanded (OpenGL/GLM).
      //
      _swap_handedness(rot);
      //
      glm::mat4 transform = glm::eulerAngleXYZ(rot.x, rot.y, rot.z);
      if (scale != 1.0)
         transform *= scale;
      transform[3] = glm::vec4(pos.x, pos.y, pos.z, 1);
      return transform;
   }
}