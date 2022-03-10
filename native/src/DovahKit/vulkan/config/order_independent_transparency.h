#pragma once
#include <cstdint>

namespace vulkanDK::config {
   // Number of fragments per pixel that can have their depths handled intelligently (additional fragments are tail-blended).
   static constexpr uint8_t oit_layer_count = 4;
}