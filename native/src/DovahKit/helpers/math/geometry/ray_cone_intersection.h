#pragma once
#include <utility>
#include "../../glm/constexpr.h"
#include "../quadratic_roots.h"
#include "../sqrt.h"
#include <glm/glm.hpp>

#include "./ray_disc_intersection.h"

namespace cobb::geometry {
   // Compute the intersection of a ray and a cone (including the base). The ray direction 
   // must be normalized. Returns the hit distance, from the ray's origin; to get the hit 
   // position, multiply that by the ray's direction and then add the ray's origin.
   constexpr bool ray_cone_intersection(
      const ::glm::vec3& ray_origin,
      const ::glm::vec3& ray_direction, // must be normalized
      
      const ::glm::vec3& cone_tip,
      const ::glm::vec3& cone_base_centerpoint,
      float cone_base_radius,

      const bool hits_from_inside_count,
      
      float& out_hit_distance
   ) {
      //
      // First, identify intersections between a line and an infinite cone. An infinite 
      // cone has no base and extends in both directions -- downward from the tip as you 
      // would expect, but also inside-out upward from the tip.
      //
      ::glm::vec3 Rl = ray_origin - cone_tip;             // Ray origin local to centerpoint
      ::glm::vec3 Cs = cone_tip - cone_base_centerpoint; // Cone spine
      float       Ch = cobb::glm::length(Cs);            // Cone height
      ::glm::vec3 Ca = Cs / Ch;                          // Cone axis
      float       Cq = (cone_base_radius * cone_base_radius) / (Ch * Ch); // Cone ratio

      auto Rd_dot_Ca = cobb::glm::dot(ray_direction, Ca);
      auto Rl_dot_Rl = cobb::glm::dot(Rl, Rl);
      auto Rl_dot_Ca = cobb::glm::dot(Rl, Ca);

      float a = cobb::glm::dot(ray_direction, ray_direction) - (Cq + 1) * (Rd_dot_Ca * Rd_dot_Ca);
      float b = 2 * (cobb::glm::dot(ray_direction, Rl) - (Cq + 1) * Rl_dot_Ca * Rd_dot_Ca);
      float c = Rl_dot_Rl - (Cq + 1) * Rl_dot_Ca * Rl_dot_Ca;

      float hit_near;
      float hit_away;
      auto  count = quadratic_roots(a, b, c, hit_near, hit_away);
      if (count == 0) {
         //
         // There is no intersection between a line (i.e. a "double-sided" ray) and the 
         // infinite cone that matches our finite cone. This means that we cannot be 
         // hitting any part of the cone: if we were hitting the base from the inside, 
         // for example, then the "back of our ray" would be hitting the upper part of 
         // the infinite cone.
         //
         return false;
      }
      //
      // Now, we need to take our intersection points and ensure that they lie on the 
      // surface of a finite cone. If one of them is below the finite cone, then we need 
      // to check for a valid intersection with the cone's base.
      //
      if (count > 2) {
         cobb::unreachable();
      }
      bool  valid1 = true;
      bool  valid2 = true;
      //
      ::glm::vec3 Hp1 = ray_origin + ray_direction * hit_near;
      ::glm::vec3 Hp2 = ray_origin + ray_direction * hit_away;
      float       Ho1 = cobb::glm::dot(cone_tip - Hp1, Ca); // height offset
      float       Ho2 = cobb::glm::dot(cone_tip - Hp2, Ca);
      //
      int valid_count = count;
      if (hit_near < 0.0 || Ho1 < 1.0e-8 || Ho1 > Ch) {
         valid1 = false;
         --valid_count;
      }
      if (hit_away < 0.0 || Ho2 < 1.0e-8 || Ho2 > Ch) {
         valid2 = false;
         if (count > 1)
            --valid_count;
      }
      //
      if (valid_count == 0) {
         if (hits_from_inside_count) {
            //
            // The ray never hits the bounded cone's curved surface. If it originates 
            // from inside the cone and points "downward," however, it could still hit 
            // the cone's endcap from inside.
            //
            if (ray_disc_intersection(ray_origin, ray_direction, cone_base_centerpoint, Ca, cone_base_radius, hit_away)) {
               out_hit_distance = hit_away;
               return true;
            }
         }
         return false;
      }
      if (valid_count == 1) {
         //
         // The ray hits the cone's curved surface only once. This can only happen under 
         // two cases: the ray originates from inside the cone, and points outward; or 
         // the ray passes through the bounded cone once and through the cone's endcap.
         //
         if (valid2) {
            Hp1 = Hp2;
            Ho1 = Ho2;
            valid1   = true;
            hit_near = hit_away;
         }
         valid2 = ray_disc_intersection(ray_origin, ray_direction, cone_base_centerpoint, Ca, cone_base_radius, hit_away);
         //
         if (!valid2) {
            if (!hits_from_inside_count) {
               //
               // The ray only hits the finite cone's curved surface once, and never hits 
               // the cone's base. This means that the ray must be originating from inside 
               // of the cone.
               //
               return false;
            }
            out_hit_distance = hit_near;
            return true;
         }
      }
      out_hit_distance = std::min(hit_near, hit_away);
      return true;
   }

   // Untested.
   extern bool ray_cone_intersection_simd(
      const ::glm::vec4& ray_origin,
      const ::glm::vec4& ray_direction, // must be normalized
      
      const ::glm::vec4& cone_tip,
      const ::glm::vec4& cone_base_centerpoint,
      float cone_base_radius,

      const bool hits_from_inside_count,
      
      float& out_hit_distance
   );
}