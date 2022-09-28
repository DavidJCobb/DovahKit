#pragma once
#include <cstdint>

namespace nifDK {
   enum class hkMotionType : uint8_t {
      invalid,
      dynamic,
      sphere_inertia,
      sphere_stabilized,
      box_inertia,
      box_stabilized,
      keyframed,
      fixed,
      thin_box,
      character,
   };
}