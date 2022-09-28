#pragma once
#include <cstdint>

namespace nifDK {
   enum class hkQualityType : uint8_t {
      invalid,
      fixed,
      keyframed,
      debris,
      moving,
      critical,
      bullet,
      user,
      character,
      keyframed_report,
   };
}