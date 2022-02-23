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
   extern void glm_transform_rotation_from_beth_in_place(float& x, float& y, float& z) {
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