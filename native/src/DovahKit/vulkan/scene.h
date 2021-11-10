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
         //
         struct {
            size_t meshes   = 0;
            size_t textures = 0;
         } pending_deletions;

         void teardown();
         void update(); // anim state, etc.

         size_t insert_new_mesh(); // returns std::string::npos on failure
         size_t insert_new_texture(); // returns std::string::npos on failure
   };
}