#pragma once
#include <cstdint>

namespace nifDK {
   enum class hkConstraintType : uint32_t {
      ball_and_socket =  0,
      hinge           =  1,
      limited_hinge   =  2,
      prismatic       =  6,
      ragdoll         =  7,
      stiff_spring    =  8,
      malleable       = 13,
   };
}