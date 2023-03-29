#pragma once
#include "../../glm/constexpr.h"
#include <glm/glm.hpp>

namespace cobb::geometry {
   constexpr bool ray_sphere_intersection(
      const ::glm::vec3& ray_origin,
      const ::glm::vec3& ray_direction, // must be normalized

      const ::glm::vec3& sphere_centerpoint,
      const float sphere_radius_squared,

      const bool hits_from_inside_count,

      ::glm::vec3& hit_position, // set only if there is a hit
      float& hit_distance // undefined (may be trashed) if there's no hit
   ) {
      constexpr auto EPSILON = 1e-8;

      auto gap = ray_origin - sphere_centerpoint;

      // 1-dimensional distance from ray origin to centerpoint, along ray
      auto ray_1D_distance_to_center = cobb::glm::dot(gap, ray_direction);
      if (ray_1D_distance_to_center < 0)
         return false; // sphere is behind ray

      // distance from sphere centerpoint to nearest point on the ray
      auto distance_squared = cobb::glm::dot(gap, gap) - ray_1D_distance_to_center * ray_1D_distance_to_center;
      if (distance_squared > sphere_radius_squared)
         return false;

      // 1-dimensional distance from sphere surface to centerpoint, along ray
      // distance is the same on either side of the center, i.e. at the ray's entry and exit points on the sphere
      float ray_1D_penetration_to_center = cobb::sqrt(sphere_radius_squared - distance_squared);

      auto distance_1D_a = ray_1D_distance_to_center - ray_1D_penetration_to_center;
      auto distance_1D_b = ray_1D_distance_to_center + ray_1D_penetration_to_center;

      if (!hits_from_inside_count) {
         if (distance_1D_a < -EPSILON || distance_1D_b < -EPSILON)
            return false;
      }

      hit_distance = distance_1D_a;
      if (distance_1D_a < EPSILON) {
         hit_distance = distance_1D_b;
      }
      if (hit_distance > EPSILON) { // if false, hit position is behind the ray
         hit_position = ray_origin + hit_distance * ray_direction;
         return true;
      }
      return false;
   }
   
   // Version that doesn't compute the hit position or distance. Good for if you're 
   // running a quick test against a bounding sphere, to see if you can skip on doing 
   // a raycast against a more complex shape.
   constexpr bool ray_intersects_sphere(
      const ::glm::vec3& ray_origin,
      const ::glm::vec3& ray_direction, // must be normalized

      const ::glm::vec3& sphere_centerpoint,
      const float sphere_radius_squared,

      const bool hits_from_inside_count
   ) {
      constexpr auto EPSILON = 1e-8;

      // ray origin, local to centerpoint location
      auto Rl = ray_origin - sphere_centerpoint;

      // 1-dimensional distance from ray origin to centerpoint, along ray
      auto Rl1D    = cobb::glm::dot(Rl, ray_direction);
      auto Rl1D_sq = Rl1D * Rl1D;
      if (!hits_from_inside_count && Rl1D_sq < sphere_radius_squared) {
         //
         // Ray originates from inside the sphere.
         //
         return false;
      }

      // distance from sphere centerpoint to nearest point on the ray
      auto distance_squared = cobb::glm::dot(Rl, Rl) - Rl1D_sq;
      if (distance_squared > sphere_radius_squared) {
         //
         // Ray never passes through the sphere.
         //
         return false;
      }

      return true;
   }
}