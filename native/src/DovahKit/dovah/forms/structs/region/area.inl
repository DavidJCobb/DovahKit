#pragma once
#include "./area.h"
#include <limits>
#include "../../../core_constants/exterior_cell_side_length.h"

namespace dovah::loaded_forms::structs::region {
   namespace impl {
      constexpr bool ranges_overlap(float u1, float u2, float v1, float v2) {
         float u_diff = u2 - u1;
         float u_max  = u_diff < 0 ? u1 : u2;
         float u_min  = u_diff < 0 ? u2 : u1;
         float v_diff = v1 - v2;
         float v_max  = v_diff > 0 ? v1 : v2;
         float v_min  = v_diff > 0 ? v2 : v1;
         if (v_min > u_max || u_min > v_max)
            return false;
         return true;
      }
      constexpr bool are_poly_edges_self_intersecting(
         const area::point& a,
         const area::point& b,
         const area::point& c,
         const area::point& d
      ) {
         if (!ranges_overlap(a.x, b.x, c.x, d.x))
            return false;
         if (!ranges_overlap(a.y, b.y, c.y, d.y))
            return false;

         const auto cd = c - d;
         const auto ba = b - a;
         const auto ac = a - c;

         float m = cd.perp_dot(ba);
         float n = ba.perp_dot(ac);
         float o = ac.perp_dot(cd);
         if (m == 0)
            return true; // `cd` and `ba` are collinear
         if (m > 0) {
            if (o >= 0)
               return false;
            if (o > m)
               return false;
            return m > n;
         } else {
            if (o >= 0)
               return false;
            if (n > 0)
               return false;
            return m <= n;
         }
      }
   }

   #pragma region area::point
      constexpr area::point area::point::operator+(const point& o) const noexcept {
         point out;
         out.x = this->x + o.x;
         out.y = this->y + o.y;
         return out;
      }
      constexpr area::point area::point::operator-(const point& o) const noexcept {
         point out;
         out.x = this->x - o.x;
         out.y = this->y - o.y;
         return out;
      }
      constexpr float area::point::perp_dot(const point& o) const noexcept {
         return (this->x * o.y) - (o.x * this->y);
      }
   #pragma endregion

   constexpr bool area::is_self_intersecting() const {
      const size_t size = this->points.size();
      if (size < 4)
         return false;
      for (size_t i = 0; i + 2 < size; ++i) {
         const auto& a = this->points[i];
         const auto& b = this->points[i + 1];
         for (size_t j = i + 2; j < size; ++j) {
            const bool is_implicitly_closing = (j + 1 >= size);
            if (is_implicitly_closing) {
               if (i == 0)
                  //
                  // For e.g. a five-vertex polygon, testing (A, B, D, A) for 
                  // self-intersection would give a false-positive result.
                  //
                  break;
            }
            const auto& c = this->points[j];
            const auto& d = is_implicitly_closing ? this->points[0] : this->points[j + 1];
            if (impl::are_poly_edges_self_intersecting(a, b, c, d))
               return true;
         }
      }
      return false;
   }
   constexpr bool area::is_valid(bool forbid_points_and_lines) const {
      if (!forbid_points_and_lines || this->points.size() >= 3) {
         return !this->is_self_intersecting();
      }
      return false;
   }

   constexpr bool area::would_become_self_intersecting(const point& next) const {
      const size_t size = this->points.size();
      if (size < 3)
         return false;

      //
      // Test all existing edges A-B against the new edges C-D formed by 
      // adding `next`. 
      //
      const auto& c = this->points[size - 1];
      const auto& d = next;
      for (size_t i = 0; i + 2 < size; ++i) {
         const auto& a = this->points[i];
         const auto& b = this->points[i + 1];
         if (impl::are_poly_edges_self_intersecting(a, b, c, d))
            return true;
      }
      return false;
   }
}