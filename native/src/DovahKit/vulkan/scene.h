#pragma once
#include <chrono>
#include <vector>
#include "loaded_texture.h"
#include "rendered_mesh.h"
#include "scene_global_state.h"

namespace vulkanDK {
   class scene {
      public:
         std::chrono::steady_clock::time_point last_update;
         std::vector<rendered_mesh>  meshes;
         std::vector<loaded_texture> textures;
         //
         scene_global_state global_state;

         void teardown();
         void update(); // anim state, etc.
   };
}