#pragma once
#include <cstdint>

namespace vulkanDK::config {
   constexpr uint32_t sun_shadow_map_resolution_x = 2048;
   constexpr uint32_t sun_shadow_map_resolution_y = 2048;
   constexpr bool     use_inverted_shadow_map     = true; // you will also have to adjust a #define in all fragment shaders that use the shadow map, to flip a comparison
}