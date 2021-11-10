#include "loaded_texture.h"

namespace vulkanDK {
   void loaded_texture::reset() {
      this->content.teardown();
      this->w = 0;
      this->h = 0;
      this->path.clear();
   }
}