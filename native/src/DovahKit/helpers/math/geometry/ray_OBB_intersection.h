#pragma once
#include "../../glm/constexpr.h"
#include <glm/glm.hpp>
#include "../abs.h"

namespace cobb::geometry {
   // Compute the intersection of a ray and an oriented bounding box. The ray direction 
   // must be normalized. Returns the hit distance, from the ray's origin; to get the hit 
   // position, multiply that by the ray's direction and then add the ray's origin.
   constexpr bool ray_OBB_intersection(
      const ::glm::vec3& ray_origin,
      const ::glm::vec3& ray_direction, // must be normalized

      const ::glm::vec3& aabb_min, // box local "minimum" corner
      const ::glm::vec3& aabb_max, // box local "maximum" corner
      
      const ::glm::mat4& box_transform, // scale is ignored

      const bool hits_from_inside_count,

      float& hit_distance // undefined (may be trashed) if there's no hit
   ) {
      constexpr float EPSILON = 1e-8;
      //
      // We treat the box's sides as planes and do a series of ray/plane intersections. A box 
      // has six sides, but these fall into one pair of parallel sides per axis.
      //
      ::glm::vec3 box_center = box_transform[3];
      ::glm::vec3 diff = box_center - ray_origin;
      //
      float nearest_entry = 0.0F;
      float furthest_exit = std::numeric_limits<float>::max();
      for (int i = 0; i < 3; ++i) {
         ::glm::vec3 axis = box_transform[i]; // normal vector for this plane

         float proj_len_ray  = cobb::glm::dot(axis, ray_direction);
         float proj_len_diff = cobb::glm::dot(axis, ::glm::vec3(box_transform[3]) - ray_origin);

         if (proj_len_ray > -EPSILON && proj_len_ray < EPSILON) {
            //
            // Ray is parallel to one of the OBB's sides.
            //
            if (-proj_len_diff + aabb_min[i] > 0.0f || -proj_len_diff + aabb_max[i] < 0.0f) {
               return false;
            }
            continue;
         }

         float entry = (proj_len_diff + aabb_min[i]) / proj_len_ray;
         float exit  = (proj_len_diff + aabb_max[i]) / proj_len_ray;
         if (entry > exit) {
            std::swap(entry, exit);
         }

         if (furthest_exit > exit) {
            furthest_exit = exit;
         }
         if (nearest_entry < entry) {
            nearest_entry = entry;
         }
         //
         // If the nearest "far" intersection is ever closer than the nearest "near" 
         // intersection, then there is no intersection.
         //
         if (furthest_exit < nearest_entry) {
            return false;
         }
      }
      if (hits_from_inside_count) {
         if (nearest_entry < 0 && furthest_exit > 0) {
            hit_distance = furthest_exit;
            return true;
         }
      }
      if (nearest_entry < 0) {
         return false;
      }
      hit_distance = nearest_entry;
      return true;
   }
}