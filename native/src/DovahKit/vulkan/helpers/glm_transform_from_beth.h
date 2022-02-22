#pragma once
#include <glm/glm.hpp>
#include "helpers/vector3.h"

namespace vulkanDK {
   extern glm::mat4 glm_transform_from_beth(const glm::vec3& pos, glm::vec3 rot, float scale);
   extern glm::mat4 glm_transform_from_beth(const cobb::vector3<float>& pos, cobb::vector3<float> rot, float scale);
}