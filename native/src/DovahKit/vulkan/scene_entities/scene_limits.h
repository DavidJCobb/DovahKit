#pragma once
#include "helpers/unreachable.h"
#include "../config/scene_limits.h"
#include "./all_classes.h"

namespace vulkanDK::scene_entities {
   // Use this for fixed, compile-time maximums on entity types.
   template<typename Entity> inline constexpr const size_t max_count_for_type = []() -> size_t {
      if constexpr (std::is_same_v<Entity, loaded_texture>) {
         return ::vulkanDK::config::max_loaded_textures;
      } else if constexpr (std::is_same_v<Entity, rendered_bounds>) {
         return ::vulkanDK::config::max_rendered_bounds;
      } else if constexpr (std::is_same_v<Entity, rendered_landscape>) {
         return ::vulkanDK::config::max_rendered_landscapes;
      } else if constexpr (std::is_same_v<Entity, rendered_light>) {
         return ::vulkanDK::config::max_rendered_lights;
      } else if constexpr (std::is_same_v<Entity, rendered_mesh>) {
         return ::vulkanDK::config::max_rendered_meshes;
      }
      cobb::unreachable();
   }();
}