#pragma once
#include "../../glm/constexpr.h"
#include "../quadratic_roots.h"
#include <glm/glm.hpp>

#include "./ray_disc_intersection.h"

namespace cobb::geometry {
   constexpr bool ray_cylinder_intersection(
      const ::glm::vec3& ray_origin,
      const ::glm::vec3& ray_direction, // must be normalized

      const ::glm::vec3& cylinder_endcap_t, // centerpoint of a disc on one end of the cylinder
      const ::glm::vec3& cylinder_endcap_b, // centerpoint of a disc on one end of the cylinder
      const float cylinder_radius, // radius of both discs

      const bool hits_from_inside_count,

      float& hit_distance
   ) {
      //
      // First, identify intersections between a line and an infinite cylinder. An infinite 
      // cylinder has no base and extends in both directions.
      //
      ::glm::vec3 Rl = ray_origin - cylinder_endcap_b;        // Ray origin local to centerpoint
      ::glm::vec3 Cs = cylinder_endcap_t - cylinder_endcap_b; // Cylinder spine
      float       Ch = cobb::glm::length(Cs);                 // Cylinder height
      ::glm::vec3 Ca = Cs / Ch;                               // Cylinder axis

      auto Ca_dot_Rd = cobb::glm::dot(Ca, ray_direction);
      auto Ca_dot_Rl = cobb::glm::dot(Ca, Rl);
      auto Rl_dot_Rl = cobb::glm::dot(Rl, Rl);

      float a = 1 - (Ca_dot_Rd * Ca_dot_Rd);
      float b = 2 * (cobb::glm::dot(ray_direction, Rl) - Ca_dot_Rd * Ca_dot_Rl);
      float c = Rl_dot_Rl - Ca_dot_Rl * Ca_dot_Rl - (cylinder_radius * cylinder_radius);

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
      bool valid1 = true;
      bool valid2 = true;
      //
      ::glm::vec3 Hp1 = ray_origin + ray_direction * hit_near;
      ::glm::vec3 Hp2 = ray_origin + ray_direction * hit_away;
      float       Ho1 = cobb::glm::dot(cylinder_endcap_t - Hp1, Ca); // height offset
      float       Ho2 = cobb::glm::dot(cylinder_endcap_t - Hp2, Ca);
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
         //
         // The ray never hits the bounded cylinder's curved surface. If we're looking 
         // along the cylinder's axis -- whether from inside or outside -- then the ray 
         // could still hit an endcap.
         // 
         // Let's project the ray origin onto the cylinder's axis, and figure out which 
         // endcap we're nearer to. (Well, actually, we already have that value: it's 
         // Ca_dot_Rl.)
         //
         if (Ca_dot_Rl <= 0.0) { // above
            valid1 = ray_disc_intersection(ray_origin, ray_direction, cylinder_endcap_t, Ca, cylinder_radius, hit_near);
         } else if (Ca_dot_Rl >= Ch) { // below
            valid1 = ray_disc_intersection(ray_origin, ray_direction, cylinder_endcap_b, Ca, cylinder_radius, hit_away);
         } else {
            //
            // Inside. Test both discs.
            //
            if (!hits_from_inside_count) {
               return false;
            }
            valid1 = ray_disc_intersection(ray_origin, ray_direction, cylinder_endcap_t, Ca, cylinder_radius, hit_near);
            valid2 = ray_disc_intersection(ray_origin, ray_direction, cylinder_endcap_b, Ca, cylinder_radius, hit_away);
            if (valid1) {
               if (valid2) {
                  if (hit_away < hit_near)
                     hit_near = hit_away;
               }
               hit_distance = hit_near;
               return true;
            } else if (valid2) {
               hit_distance = hit_away;
               return true;
            }
         }
         if (valid1) {
            hit_distance = hit_near;
            return true;
         }
         return false;
      }
      if (valid_count == 1) {
         //
         // The ray hits the cylinder's curved surface only once. This can only happen under 
         // two cases: the ray originates from inside the cylinder, and points outward; or 
         // the ray passes through the bounded cylinder once and then through an endcap.
         //
         if (valid2) {
            Hp1 = Hp2;
            Ho1 = Ho2;
            valid1 = true;
            hit_near = hit_away;
         }

         float disc_near;
         float disc_away;
         bool  disc1 = ray_disc_intersection(ray_origin, ray_direction, cylinder_endcap_t, Ca, cylinder_radius, disc_near);
         bool  disc2 = ray_disc_intersection(ray_origin, ray_direction, cylinder_endcap_b, Ca, cylinder_radius, disc_away);
         if (disc1) {
            if (disc2) {
               if (disc_away < disc_near)
                  disc_near = disc_away;
            }
         } else if (disc2) {
            disc_near = hit_away;
            disc1 = disc2;
         }

         if (disc1) {
            if (disc_near < hit_near) {
               hit_distance = disc_near;
               return true;
            }
         } else {
            if (!hits_from_inside_count) {
               //
               // The ray only hits the finite cone's curved surface once, and never hits 
               // the cone's base. This means that the ray must be originating from inside 
               // of the cone.
               //
               return false;
            }
            hit_distance = hit_near;
            return true;
         }
      }
      hit_distance = std::min(hit_near, hit_away);
      return true;
   }
}