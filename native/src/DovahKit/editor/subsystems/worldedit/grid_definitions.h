#pragma once
#include "helpers/math/sqrt.h"
#include "helpers/resizable_grid.h"
#include "vulkan/config/scene_limits.h"

namespace dovahkit::subsystems::worldedit {
   constexpr const size_t max_loaded_grid_size = cobb::sqrt(vulkanDK::config::max_rendered_landscapes);

   using loaded_cell_grid_coord = int8_t;
}

#include <limits>
namespace dovahkit::subsystems::worldedit {
   static_assert(
      max_loaded_grid_size <= std::numeric_limits<loaded_cell_grid_coord>::max() &&
      -max_loaded_grid_size >= std::numeric_limits<loaded_cell_grid_coord>::lowest(),
      "Not all valid grid coordinates are representable in the type chosen for loaded cell grid coordinates."
   );
}