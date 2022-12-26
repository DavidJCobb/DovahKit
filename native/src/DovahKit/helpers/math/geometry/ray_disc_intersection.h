#pragma once
#include "../../glm/constexpr.h"
#include <glm/glm.hpp>
#include "./ray_plane_intersection.h"

namespace cobb::geometry {
   // Compute the intersection of a ray and a disc. The ray direction must be normalized. 
   // Returns the hit distance, from the ray's origin.
   constexpr bool ray_disc_intersection(
      const ::glm::vec3& ray_origin,
      const ::glm::vec3& ray_direction, // must be normalized

      const ::glm::vec3& disc_origin,
      const ::glm::vec3& disc_normal,
      float radius,

      float& hit_distance
   ) {
      float Hd;
      bool  plane = ray_plane_intersection(ray_origin, ray_direction, disc_origin, disc_normal, Hd);
      if (!plane)
         return false;
      ::glm::vec3 Hp = ray_origin + ray_direction * Hd;
      ::glm::vec3 Dd = Hp - disc_origin;
      if (cobb::glm::dot(Dd, Dd) > radius * radius)
         return false;
      //if (Hd < 0) // redundant with checks done in ray/plane
      //   return false;
      hit_distance = Hd;
      return true;
   }
}