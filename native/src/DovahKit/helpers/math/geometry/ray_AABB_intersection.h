#pragma once
#include "../../glm/constexpr.h"
#include <glm/glm.hpp>

namespace cobb::geometry {
   // Compute the intersection of a ray and an axis-aligned bounding box. The ray direction 
   // must be normalized. Returns the hit distance, from the ray's origin; to get the hit 
   // position, multiply that by the ray's direction and then add the ray's origin.
   constexpr bool ray_AABB_intersection(
      const ::glm::vec3& ray_origin,
      const ::glm::vec3& ray_direction, // must be normalized

      const ::glm::vec3& aabb_min, // box local "minimum" corner
      const ::glm::vec3& aabb_max, // box local "maximum" corner
      
      const ::glm::vec3& box_centerpoint,

      const bool hits_from_inside_count,

      float& hit_distance // undefined (may be trashed) if there's no hit
   ) {
      //
      // We treat the box's sides as planes and do a series of ray/plane intersections. A box 
      // has six sides, but these fall into one pair of parallel sides per axis.
      //
      auto ray_inv = ::glm::vec3{ 1.0 / ray_direction.x, 1.0 / ray_direction.y, 1.0 / ray_direction.z };

      // We're going to start with the math for a line/AABB intersection check. The line will 
      // intersect with the box twice. We define the "entry" and "exit" points relative to the 
      // ray origin and direction, but again, it's a line/box check and so will hit boxes that 
      // are behind the ray.
      float entry = std::numeric_limits<float>::max();
      float exit  = std::numeric_limits<float>::lowest();

      float a = 0;
      float b = 0;
      for (size_t i = 0; i < 3; ++i) {
         a = ((aabb_min[i] + box_centerpoint[i]) - ray_origin[i]) * ray_inv[i];
         b = ((aabb_min[i] + box_centerpoint[i]) - ray_origin[i]) * ray_inv[i];
         entry = std::max(entry, std::min(a, b));
         exit  = std::min(entry, std::max(a, b));
      }

      if (entry < exit) {
         return false;
      }
      if (!hits_from_inside_count) {
         if (entry < 0) {
            return false; // entry point is behind the ray (and therefore the ray is inside the box)
         }
      }
      if (exit <= 0) {
         return false; // exit point (and therefore the box as a whole) is behind the ray
      }

      hit_distance = entry;
      return true;
   }
}