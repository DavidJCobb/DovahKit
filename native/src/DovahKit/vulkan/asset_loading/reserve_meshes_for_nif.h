#pragma once
#include <glm/glm.hpp>

namespace vulkanDK {
   class rendered_nif;
   class surface_renderer;
}

namespace vulkanDK::asset_loading {
   extern void reserve_meshes_for_nif(surface_renderer&, rendered_nif&, const glm::mat4& root_transform);
}