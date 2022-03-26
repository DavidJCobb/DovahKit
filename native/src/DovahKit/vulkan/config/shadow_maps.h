#pragma once
#include <cstdint>

namespace vulkanDK::config {
   constexpr bool use_inverted_shadow_map = false; // you will also have to adjust a #define in all fragment shaders that use the shadow map, to flip a comparison

   // Settings for the sun:
   constexpr uint32_t sun_shadow_map_resolution_x = 2048;
   constexpr uint32_t sun_shadow_map_resolution_y = 2048;
   constexpr bool     sun_shadow_invert_culling   = true; // if enabled, front-face culling is used to prevent depth biasing from causing floating shadows ("peter-panning")

   // Settings for placed lights:
   constexpr uint32_t light_shadow_map_resolution_x = 1024;
   constexpr uint32_t light_shadow_map_resolution_y = 1024;
   constexpr bool     light_shadow_invert_culling   = false; // if enabled, front-face culling is used to prevent depth biasing from causing floating shadows ("peter-panning")
}