#pragma once
#include <array>
#include <cstdint>

namespace vulkanDK {
   template<size_t w, size_t h> requires (w > 1 && h > 1)
   extern constexpr size_t vertex_index_count_for_quad_grid_outline = ([]() {
      return w + (h - 2) + w + (h - 2) + 1; // +1 because it must loop back to and include the starting index
   })();

   template<size_t w, size_t h> requires (w > 1 && h > 1)
   extern constexpr std::array<uint16_t, vertex_index_count_for_quad_grid_outline<w, h>> vertex_indices_for_quad_grid_outline = ([]() {
      std::array<uint16_t, vertex_index_count_for_quad_grid_outline<w, h>> indices = {};
      //
      size_t i = 0;
      //
      for (size_t x = 0; x < w; ++x) { // from upper-left corner to and including upper-right corner; left to right
         indices[i++] = x;
      }
      for (size_t y = 1; y < h - 1; ++y) { // between the upper-right and lower-right corners; top to bottom
         indices[i++] = (y * w) + (w - 1);
      }
      for (size_t x = 0; x < w; ++x) { // from the lower-right to and including the lower-left corner; right to left
         indices[i++] = ((h - 1) * w) + (w - x - 1);
      }
      for (size_t y = 1; y < h - 1; ++y) { // between the lower-left and upper-left corners; bottom to top
         indices[i++] = ((h - y - 1) * w);
      }
      indices[i++] = 0; // and then return to where we started
      //
      return indices;
   })();
}