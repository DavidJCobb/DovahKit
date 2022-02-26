#pragma once
#include <glm/glm.hpp>

namespace vulkanDK {
   struct scene_shadow_state {
      alignas(16) glm::vec3 sun_dir   = glm::normalize(glm::vec3{ 0.1, 0, -1 });
      alignas(16) glm::vec3 sun_pos   = {};
      alignas(16) glm::mat4 sun_space = glm::mat4(1);
   };
}