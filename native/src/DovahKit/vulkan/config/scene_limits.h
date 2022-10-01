#pragma once

namespace vulkanDK::config {
   static constexpr size_t max_rendered_bounds     =  1000;
   static constexpr size_t max_rendered_landscapes =    30 * 30;
   static constexpr size_t max_rendered_lights     =   240;
   static constexpr size_t max_rendered_meshes     = 32000;
   static constexpr size_t max_loaded_textures     =  2000;

   static constexpr size_t max_active_shadow_casters = 4;

   static constexpr size_t initial_landscape_side_count = 5;
}
