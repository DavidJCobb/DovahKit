#include "glm_transform_from_beth.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace vulkanDK {
   extern glm::vec3 glm_transform_rotation_from_beth(const glm::vec3& rot) {
      return rot * -1.0F;
   }
   extern glm::vec3 glm_transform_rotation_from_beth(const cobb::vector3<float>& rot) {
      return glm::vec3{ -rot.x, -rot.y, -rot.z };
   }
   extern void glm_transform_rotation_from_beth_in_place(glm::vec3& rot) {
      rot *= -1.0F;
   }

   extern glm::mat4 glm_transform_from_beth(const glm::vec3& pos, glm::vec3 rot, float scale) {
      glm_transform_rotation_from_beth_in_place(rot);
      //
      glm::mat4 transform;
      if constexpr (vulkanDK::config::bethesda_rotation_order_zyx) {
         transform = glm::eulerAngleZYX(rot.x, rot.y, rot.z);
      } else {
         transform = glm::eulerAngleXYZ(rot.x, rot.y, rot.z);
      }
      if (scale != 1.0)
         transform *= scale;
      transform[3] = glm::vec4(pos, 1);
      return transform;
   }
   extern glm::mat4 glm_transform_from_beth(const cobb::vector3<float>& pos, cobb::vector3<float> rot, float scale) {
      return glm_transform_from_beth(pos, glm_transform_rotation_from_beth(rot), scale);
   }
}