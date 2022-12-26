#pragma once
#include <utility>
#include "./sqrt.h"

namespace cobb {
   //
   // Given coefficients in a quadratic equation, this function gives you the roots 
   // and returns the number of roots. If there is only one root, then both root 
   // variables are set to the same value.
   //
   constexpr int quadratic_roots(
      const float a,
      const float b,
      const float c,
      //
      float& root_lower,
      float& root_upper
   ) {
      constexpr auto EPSILON = 1e-8;

      float discriminant = (b * b) - (4.0 * a * c);
      if (discriminant > EPSILON) {
         float b_term = b < EPSILON ? -b + cobb::sqrt(discriminant) : -b - cobb::sqrt(discriminant);

         root_lower = b_term / (2.0 * a); // quadratic formula
         root_upper = (2.0 * c) / b_term; // citardauq formula

         if (root_lower > root_upper)
            std::swap(root_lower, root_upper); // use of both formulae, plus this, avoids catastrophic cancellation due to floating-point limits

         return 2;
      } else if (discriminant > -EPSILON && discriminant <= EPSILON) {
         root_lower = -(b / 2.0 * a);
         root_upper = root_lower;
         return 1;
      }
      root_lower = NAN;
      root_upper = NAN;
      return 0;
   }
}