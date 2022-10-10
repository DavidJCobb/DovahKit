#pragma once
#include <limits>
#include <variant>
#include <glm/glm.hpp>
#include "./scene_entity_handle.h"

namespace vulkanDK {
   class surface_renderer;
}

namespace vulkanDK {
   struct raycast_hit_data {
      glm::vec2 bary_position;
      float     distance = std::numeric_limits<float>::max();
      glm::vec3 position;
      glm::vec3 surface_normal;

      operator bool() const noexcept { return this->distance < std::numeric_limits<float>::max(); }
   };

   class raycast {
      public:
         raycast(surface_renderer& sr) : owner(sr) {}

         surface_renderer& owner;
         glm::vec3 origin    = { 0.0, 0.0, 0.0 };
         glm::vec3 direction = { 0.0, 0.0, 1.0 };

         struct {
            std::variant<
               std::monostate,
               rendered_landscape_handle,
               rendered_mesh_handle
            > entity;
            raycast_hit_data hit;
         } result;

         // Returns true if the passed-in hit is nearer to the raycast's primary stored hit, 
         // and so has *become* the raycast's primary stored hit.
         bool receive_hit(const raycast_hit_data&);

         void set_screen_relative_raycast(int viewport_x, int viewport_y);
   };
}