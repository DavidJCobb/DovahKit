#pragma once
#include <cstdint>

namespace vulkanDK::config {
   static constexpr struct {
      uint8_t supersample = 1; // 1 == just render the scene normally; else == render the scene at a multiplied resolution; then downscale
      uint8_t multisample = 1; // 1 == just render the scene normally; else == supersample just the depth/stencil buffer
   } antialias_mode;
}