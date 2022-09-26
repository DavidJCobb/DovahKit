#pragma once
#include <type_traits>
#include "../../helpers/class_array.h"
#include "../loaded_texture.h"
#include "../rendered_bounds.h"
#include "../rendered_landscape.h"
#include "../rendered_light.h"
#include "../rendered_mesh.h"

namespace vulkanDK::scene_entities {
   using all_types = cobb::class_array<
      loaded_texture,
      rendered_bounds,
      rendered_landscape,
      rendered_light,
      rendered_mesh//,
   >;
}

#include "./concepts/has_frame_culling_data.h"
#include "./concepts/has_frame_drawing_data.h"

namespace vulkanDK::scene_entities {
   using all_types_with_frame_culling_data = all_types::filter_types<[]<typename T>() -> bool {
      return concepts::has_frame_culling_data<T>;
   }>;
   using all_types_with_frame_drawing_data = all_types::filter_types<[]<typename T>() -> bool {
      return concepts::has_frame_drawing_data<T>;
   }>;
   using all_types_that_are_drawn = all_types::filter_types<[]<typename T>() -> bool {
      return T::is_drawn;
   }>;
}