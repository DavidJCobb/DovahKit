#include "scene.h"

namespace vulkanDK {
   void scene::teardown() {
      this->meshes.clear();
      this->textures.clear();
   }
}