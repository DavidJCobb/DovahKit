#pragma once
#include "../../glm/constexpr.h"
#include <glm/glm.hpp>

namespace cobb::geometry {
   //
   // Implementation of the Moller-Trumbore method for computing ray/triangle 
   // intersections. Uses barycentric coordinates to do it. Think of those as 
   // like alpha-blending, but instead of single values, we're blending three 
   // vectors using two "alphas."
   //
   constexpr bool ray_triangle_intersection(
      const ::glm::vec3& ray_origin,
      const ::glm::vec3& ray_direction, // must be normalized

      const ::glm::vec3& Ta, // triangle vertex
      const ::glm::vec3& Tb, // triangle vertex
      const ::glm::vec3& Tc, // triangle vertex

      bool allow_backface_hits,

      ::glm::vec2& out_barycentric,
      float& out_hit_distance
   ) {
      constexpr float EPSILON = 1e-8;

      auto E1 = Tb - Ta;
      auto E2 = Tc - Ta;

      auto influence_u = cobb::glm::cross(ray_direction, E2);
      auto determinant = cobb::glm::dot(influence_u, E1);
      if (allow_backface_hits) {
         if (determinant > -EPSILON && determinant < EPSILON)
            return false;
      } else {
         if (determinant < EPSILON)
            return false;
      }

      float u;
      float v;

      auto Rl = ray_origin - Ta;

      u = cobb::glm::dot(influence_u, Rl) / determinant;
      if (u < 0 || u > 1)
         return false;
      auto influence_v = cobb::glm::cross(Rl, E1);
      v = cobb::glm::dot(influence_v, ray_direction) / determinant;
      if (v < 0 || u + v > 1)
         return false;

      out_barycentric  = ::glm::vec2{ u, v };
      out_hit_distance = cobb::glm::dot(influence_v, E2) / determinant;
      return true;
   }
}