#pragma once
#include <glm/glm.hpp>

namespace cobb::glm {
   extern ::glm::vec4 cross_simd(const ::glm::vec4&, const ::glm::vec4&);
   extern __m128 cross_simd_to_register(const ::glm::vec4&, const ::glm::vec4&);
}