#pragma once
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