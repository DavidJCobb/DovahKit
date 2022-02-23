#pragma once
#include <glm/glm.hpp>

namespace nifDK {
   // NOTE: NiMatrix33 is row-major in Skyrim's RAM, but glm::mat3 is column-major.
   using NiMatrix33 = glm::mat3; // mat[col][row]
}