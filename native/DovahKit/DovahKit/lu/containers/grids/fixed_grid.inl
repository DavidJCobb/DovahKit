#pragma once
#include "./fixed_grid.h"
#include <cassert>

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#define TEMPLATE_PARAMS template<typename T, size_t w, size_t h, signed int offset_x, signed int offset_y>
#define CLASS_NAME fixed_grid<T, w, h, offset_x, offset_y>

namespace lu {
   TEMPLATE_PARAMS
   constexpr const CLASS_NAME::value_type* CLASS_NAME::at(coordinate_type x, coordinate_type y) const {
      auto i = coordinates_to_index(x, y);
      if (i == no_valid_index)
         return nullptr;
      return &this->_data[i];
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::value_type* CLASS_NAME::at(coordinate_type x, coordinate_type y) {
      auto i = coordinates_to_index(x, y);
      if (i == no_valid_index)
         return nullptr;
      return &this->_data[i];
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::clear() {
      for (auto& item : this->_data) {
         item = {};
      }
   }

   TEMPLATE_PARAMS
   /*static*/ constexpr bool CLASS_NAME::coordinates_in_bounds(coordinate_type x, coordinate_type y) {
      if (x < left)
         return false;
      if (y < top)
         return false;
      if (size_t(x - left) >= width)
         return false;
      if (size_t(y - top) >= height)
         return false;
      return true;
   }

   TEMPLATE_PARAMS
   /*static*/ constexpr size_t CLASS_NAME::coordinates_to_index(coordinate_type x, coordinate_type y) {
      if (x < left || y < top)
         return no_valid_index;
      if constexpr (left != 0)
         x -= left;
      if constexpr (top != 0)
         y -= top;
      if (x >= width || y >= height)
         return no_valid_index;
      return (size_t)x + ((size_t)y * width);
   }

   TEMPLATE_PARAMS
   static constexpr bool CLASS_NAME::flat_index_in_bounds(size_t i) {
      return i < area;
   }
   
   #pragma region item
      TEMPLATE_PARAMS
      constexpr const CLASS_NAME::value_type& CLASS_NAME::item(coordinate_type x, coordinate_type y) const {
         const auto i = coordinates_to_index(x, y);
         assert(i != no_valid_index);
         return this->_data[i];
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type& CLASS_NAME::item(coordinate_type x, coordinate_type y) {
         const auto i = coordinates_to_index(x, y);
         assert(i != no_valid_index);
         return this->_data[i];
      }
   #pragma endregion
   #pragma region item_by_flat_index
      TEMPLATE_PARAMS
      constexpr const CLASS_NAME::value_type& CLASS_NAME::item_by_flat_index(size_t i) const {
         assert(i < area);
         return this->_data[i];
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type& CLASS_NAME::item_by_flat_index(size_t i) {
         assert(i < area);
         return this->_data[i];
      }
      
      TEMPLATE_PARAMS
      template<size_t i>
      constexpr const CLASS_NAME::value_type& CLASS_NAME::item_by_flat_index() const noexcept requires (i < area) {
         return this->_data[i];
      }

      TEMPLATE_PARAMS
      template<size_t i>
      constexpr CLASS_NAME::value_type& CLASS_NAME::item_by_flat_index() noexcept requires (i < area) {
         return this->_data[i];
      }
   #pragma endregion

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::list_type& CLASS_NAME::list() noexcept { return this->_data; }

   TEMPLATE_PARAMS
   constexpr const CLASS_NAME::list_type& CLASS_NAME::list() const noexcept { return this->_data; }
}

#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")