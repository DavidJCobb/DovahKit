#pragma once
#include <glm/glm.hpp>

namespace dovah::loaded_forms::components {
   class model;
}
namespace vulkanDK {
   class rendered_nif;
}

namespace vulkanDK::asset_loading {
   struct queued_nif_load {
      dovah::loaded_forms::components::model* form_data = nullptr;
      rendered_nif* nif = nullptr;
      glm::mat4     transform;
   };
}
