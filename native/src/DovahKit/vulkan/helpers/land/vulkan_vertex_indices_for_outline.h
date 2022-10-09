#pragma once
#include <array>
#include <cstdint>
#include "dovah/forms/Landscape.h"
#include "./vertex_index_conversions.h"

namespace vulkanDK::helpers::land {
   static constexpr auto line_vertex_indices = [](){
      using loaded_form = dovah::loaded_forms::Landscape;
      constexpr const size_t w = loaded_form::vertices_per_side;
      constexpr const size_t h = loaded_form::vertices_per_side;
      //
      const auto _quad_of = [](size_t x, size_t y) constexpr -> size_t {
         size_t q = 0;
         q += (x >= centerline_rowcol_index) ? 1 : 0;
         q += (y >= centerline_rowcol_index) ? 2 : 0;
         return q;
      };
      const auto _vulkan_index_of = [_quad_of](size_t land_x, size_t land_y) constexpr -> size_t {
         auto q = _quad_of(land_x, land_y);
         return vulkan_vertex_index_for_per_land_vertex_index(q, land_x + (land_y * loaded_form::vertices_per_side));
      };

      std::array<uint16_t, (w + (h - 2)) * 2 + 1> indices = {};
      //
      size_t i = 0;
      for (size_t x = 0; x < w; ++x) {
         indices[i++] = _vulkan_index_of(x, 0);
      }
      for (size_t y = 1; y < h - 1; ++y) {
         indices[i++] = _vulkan_index_of(w - 1, y);
      }
      for (size_t x = 0; x < w; ++x) {
         indices[i++] = _vulkan_index_of(w - x - 1, h - 1);
      }
      for (size_t y = 1; y < h - 1; ++y) {
         indices[i++] = _vulkan_index_of(0, h - y - 1);
      }
      indices[i++] = indices[0];
      //
      return indices;
   }();

   static_assert(
      []() -> bool {
         const auto& list = line_vertex_indices;
         //
         size_t start_count = 0;
         for (auto index : list)
            if (index == list[0])
               ++start_count;
         return start_count == 2;
      }(),
      "The list must start and end at the same vertex index."
   );
   static_assert(
      []() -> bool {
         const auto& list = line_vertex_indices;
         //
         for (size_t i = 1; i + 1 < list.size(); ++i) {
            for (size_t j = i + 1; j < list.size(); ++j) {
               if (list[i] == list[j])
                  return false;
            }
         }
         return true;
      }(),
      "The list must not contain duplicate indices besides the start-/endpoint."
   );
}