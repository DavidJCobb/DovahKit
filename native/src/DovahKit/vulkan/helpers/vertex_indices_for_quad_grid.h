#pragma once
#include <array>
#include <cstdint>

namespace vulkanDK {
   template<size_t w, size_t h> extern constexpr size_t vertex_index_count_for_quad_grid = ([]() {
      static_assert(w > 1 && h > 1, "If the width or height is only 1, then what you are asking for is not a grid. It is a line.");
      //
      constexpr auto quad_count    = (w - 1) * (h - 1);
      constexpr auto tri_count     = quad_count * 2;
      constexpr auto verts_per_tri = 3;
      //
      return tri_count * verts_per_tri;
   })();

   template<size_t w, size_t h, bool clockwise = true> extern constexpr std::array<uint16_t, vertex_index_count_for_quad_grid<w, h>> vertex_indices_for_quad_grid = ([]() {
      std::array<uint16_t, vertex_index_count_for_quad_grid<w, h>> indices = {};
      //
      size_t i = 0;
      for (size_t y = 0; y + 1 < h; ++y) {
         for (size_t x = 0; x + 1 < w; ++x) {
            size_t a = y * w + x;
            size_t b = a + 1;
            size_t c = a + w;
            size_t d = c + 1;
            if constexpr (clockwise) {
               // tri 1:
               indices[i++] = a;
               indices[i++] = b;
               indices[i++] = c;
               // tri 2:
               indices[i++] = b;
               indices[i++] = d;
               indices[i++] = c;
            } else {
               // tri 1:
               indices[i++] = a;
               indices[i++] = c;
               indices[i++] = b;
               // tri 2:
               indices[i++] = b;
               indices[i++] = c;
               indices[i++] = d;
            }
         }
      }
      return indices;
   })();
}