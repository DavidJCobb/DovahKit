#pragma once
#include "../config/scene_limits.h"
#include "./all_classes.h"

namespace vulkanDK::scene_entities {
   // Use this for fixed, compile-time maximums on entity types.
   template<typename Entity> inline constexpr const size_t max_count_for_type = []() -> size_t {
      if constexpr (std::is_same_v<Entity, rendered_bounds>) {
         return ::vulkanDK::config::max_rendered_bounds;
      } else if constexpr (std::is_same_v<Entity, rendered_light>) {
         return ::vulkanDK::config::max_rendered_lights;
      } else if constexpr (std::is_same_v<Entity, rendered_mesh>) {
         return ::vulkanDK::config::max_rendered_meshes;
      }
      return 0; // indicates no maximum or a maximum determined at run-time
   }();

   // Use this when an entity type's maximum can vary during run-time; this 
   // sets an initial maximum so that renderer setup can work.
   template<typename Entity> inline constexpr const size_t initial_cap_for_type = []() -> size_t {
      if constexpr (std::is_same_v<Entity, rendered_landscape>) {
         return ::vulkanDK::config::initial_landscape_side_count * ::vulkanDK::config::initial_landscape_side_count;
      }
      return max_count_for_type<Entity>;
   }();
}