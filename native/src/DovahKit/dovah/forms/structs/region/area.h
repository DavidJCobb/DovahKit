#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace dovah::loaded_forms::structs::region {
   struct area {
      public:
         union point {
            constexpr point() : x(0), y(0) {}
            constexpr ~point() {}
            struct {
               float x;
               float y;
            };
            std::array<float, 2> list;

            constexpr point operator+(const point& o) const noexcept;
            constexpr point operator-(const point& o) const noexcept;
            constexpr float perp_dot(const point& o) const noexcept;
         };

      public:
         uint32_t edge_falloff = 512; // RPLI
         std::vector<point> points; // RPLD[]

      public:
         constexpr bool is_self_intersecting() const;
         constexpr bool is_valid(bool forbid_points_and_lines) const;

         // Assumes the area is not already self-intersecting.
         constexpr bool would_become_self_intersecting(const point&) const;
   };
}

#include "./area.inl"