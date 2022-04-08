#pragma once
#include <array>
#include <glm/glm.hpp>

namespace vulkanDK {
   extern std::array<glm::vec4, 4> extract_frustum_normals(const glm::mat4& view_proj);
}