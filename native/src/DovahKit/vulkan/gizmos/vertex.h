#pragma once
#include <glm/glm.hpp>

namespace vulkanDK::gizmos {
   struct vertex {
      glm::vec4 position = { 0, 0, 0, 0 }; // W-component identifies the axis (0, 1, 2 == X, Y, Z)

      constexpr vertex() {}
      constexpr vertex(const glm::vec3& v) : position(v, 0) {}
   };
}