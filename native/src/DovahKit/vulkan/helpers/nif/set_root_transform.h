#pragma once
#include <glm/glm.hpp>

namespace vulkanDK {
   class rendered_nif;
}

namespace vulkanDK::helpers::nif {
   extern void set_root_transform(rendered_nif&, const glm::mat4& transform);
}
