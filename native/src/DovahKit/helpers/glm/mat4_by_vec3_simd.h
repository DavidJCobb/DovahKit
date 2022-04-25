#pragma once
#include <glm/glm.hpp>

namespace cobb::glm {
   // Identical to mat4_by_vec4_simd, but saves us a shuffle for the W-component.
   extern ::glm::vec4 mat4_by_vec3_simd(const ::glm::mat4&, const ::glm::vec3&);
}