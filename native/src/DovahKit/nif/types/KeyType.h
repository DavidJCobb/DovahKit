#pragma once
#include <cstdint>

namespace nifDK {
   enum class KeyType : uint32_t {
     linear       = 1,
     quadratic    = 2,
     tension_bias_continuity = 3,
     xyz_rotation = 4,
     constant     = 5,

     // NifSkope names:
     LINEAR_KEY = linear,
     QUADRATIC_KEY = quadratic,
     TBC_KEY = tension_bias_continuity,
     XYZ_ROTATION_KEY = xyz_rotation,
     CONST_KEY = constant,
   };
}