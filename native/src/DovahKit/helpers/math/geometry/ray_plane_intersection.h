#pragma once
#include "../../glm/constexpr.h"
#include <glm/glm.hpp>

namespace cobb::geometry {
   // Compute the intersection of a ray and a plane with infinite bounds. The ray direction 
   // must be normalized. Returns the hit distance, from the ray's origin; to get the hit 
   // position, multiply that by the ray's direction and then add the ray's origin.
   constexpr bool ray_plane_intersection(
      const ::glm::vec3& ray_origin,
      const ::glm::vec3& ray_direction, // must be normalized

      const ::glm::vec3& plane_origin,
      const ::glm::vec3& plane_normal,

      float& hit_distance
   ) {
      constexpr float EPSILON = 1e-8;
      //
      // A point P is on the plane if dot(P - plane_origin, plane_normal) == 0.
      //
      // A point P is on the line if, given some distance D: P == ray_origin + ray_direction * D.
      //
      // Therefore, we have an intersection if:
      // 
      //    dot(ray_origin + ray_direction * D - plane_origin, plane_normal) == 0.
      // 
      // Unravel the dot product and solve for D to get the math here. Then, apply D to the line to 
      // get the hit position.
      // 
      auto denom = cobb::glm::dot(plane_normal, ray_direction);
      if (denom > EPSILON || denom < -EPSILON) {
         auto Hd = cobb::glm::dot(plane_origin - ray_origin, plane_normal) / denom;
         if (Hd >= 0) {
            hit_distance = Hd;
            return true;
         }
      }
      return false;
   }
}