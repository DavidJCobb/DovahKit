#include "raycast.h"
#include "./surface_renderer.h"

namespace vulkanDK {
   bool raycast::receive_hit(const raycast_hit_data& hit) {
      if (!hit)
         return false;
      auto& prior = this->result.hit;
      if (hit.distance >= prior.distance)
         return false;
      prior = hit;
      return true;
   }
   void raycast::set_screen_relative_raycast(int viewport_x, int viewport_y) {
      this->owner.surface_position_to_world_ray(
         viewport_x,
         viewport_y,
         this->origin,
         this->direction
      );
   }
}