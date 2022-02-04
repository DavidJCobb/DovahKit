#pragma once
#include <array>
#include <cstdint>

namespace nifDK {
   struct Triangle {
      std::array<uint16_t, 3> vertex_indices;
   };
}