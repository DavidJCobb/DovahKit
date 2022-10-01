#pragma once

namespace vulkanDK::config {
   static constexpr size_t max_rendered_bounds = 1000;
   static constexpr size_t max_rendered_lights =  100;
   static constexpr size_t max_rendered_meshes = 8192;
   static constexpr size_t max_loaded_textures = 1600;

   static constexpr size_t max_active_shadow_casters = 4;

   static constexpr size_t initial_landscape_side_count = 5;
}
