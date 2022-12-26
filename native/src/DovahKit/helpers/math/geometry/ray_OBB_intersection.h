#pragma once
#include "../../glm/constexpr.h"
#include <glm/glm.hpp>
#include "../sqrt.h"

namespace cobb::geometry {
   // Compute the intersection of a ray and an oriented bounding box. The ray direction 
   // must be normalized. Returns the hit distance, from the ray's origin; to get the hit 
   // position, multiply that by the ray's direction and then add the ray's origin.
   constexpr bool ray_OBB_intersection(
      const ::glm::vec3& ray_origin,
      const ::glm::vec3& ray_direction, // must be normalized

      const ::glm::vec3& aabb_min, // box local "minimum" corner
      const ::glm::vec3& aabb_max, // box local "maximum" corner
      
      const ::glm::vec3& box_origin,
      const ::glm::mat4& box_transform, // scale is ignored

      const bool hits_from_inside_count,

      float& hit_distance // undefined (may be trashed) if there's no hit
   ) {
      //
      // Based on methods presented in "An Efficient and Robust Ray–Box Intersection Algorithm", 
      // by Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley.
      //
      // We treat the box's sides as planes and do a series of ray/plane intersections. A box 
      // has six sides, but these fall into one pair of parallel sides per axis.
      //
      ::glm::vec3 box_center = box_transform[3];
      ::glm::vec3 diff = box_center - ray_origin;
      //
      float greatest_min = 0.0F;
      float smallest_max = std::numeric_limits<float>::max();
      for (int i = 0; i < 3; ++i) {
         ::glm::vec3 axis = box_transform[i];
         float unscaled_hit = glm::dot(axis, diff);
         float denom        = glm::dot(ray_direction, axis);
         if (fabs(denom) > 1e-8) {
            //
            // For the current axis, compute both ray/plane intersections; e.g. for the X-axis, 
            // compute the intersections with the YZ plane.
            //
            float behind = (unscaled_hit + aabb_min[i]) / denom;
            float ahead  = (unscaled_hit + aabb_max[i]) / denom;
            if (behind > ahead) {
               std::swap(behind, ahead);
            }
            //
            if (smallest_max > ahead)
               smallest_max = ahead;  // nearest exit point for any plane
            if (greatest_min < behind)
               greatest_min = behind; // furthest entry point for any plane
            //
            // If the nearest "far" intersection is ever closer than the nearest "near" 
            // intersection, then there is no intersection.
            //
            if (smallest_max < greatest_min)
               return false;
         } else {
            //
            // Ray is parallel to the AABB.
            //
            if (-unscaled_hit + aabb_min[i] > 0.0f || -unscaled_hit + aabb_max[i] < 0.0f)
               return false;
         }
      }
      hit_distance = greatest_min;
      return true;
   }
}