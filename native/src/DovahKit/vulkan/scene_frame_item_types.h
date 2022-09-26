#pragma once
#include <type_traits>
#include "../helpers/class_array.h"
#include "./loaded_texture.h"
#include "./rendered_bounds.h"
#include "./rendered_landscape.h"
#include "./rendered_light.h"
#include "./rendered_mesh.h"

#include "./config/scene_limits.h"

namespace vulkanDK {
   using all_scene_frame_item_types = cobb::class_array<
      loaded_texture,
      rendered_bounds,
      rendered_landscape,
      rendered_light,
      rendered_mesh//,
   >;

   template<typename SFI> inline constexpr const size_t max_scene_frame_item_count = [](){
      if constexpr (std::is_same_v<SFI, rendered_bounds>) {
         return config::max_rendered_bounds;
      } else if constexpr (std::is_same_v<SFI, rendered_landscape>) {
         return config::max_rendered_landscapes;
      } else if constexpr (std::is_same_v<SFI, rendered_light>) {
         return config::max_rendered_lights;
      } else if constexpr (std::is_same_v<SFI, rendered_mesh>) {
         return config::max_rendered_meshes;
      }
      return 0; // indicates no maximum or a maximum determined at run-time
   }();
}