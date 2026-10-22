#pragma once
#include <array>
#include <cstdint>
#include <limits>

namespace lu {
   template<typename T, size_t w, size_t h, signed int offset_x = 0, signed int offset_y = 0>
   class fixed_grid {
      public:
         static constexpr size_t width  = w;
         static constexpr size_t height = h;
         static constexpr size_t area   = w * h;

         using value_type      = T;
         using list_type       = std::array<value_type, area>;
         using coordinate_type = int32_t;

         static constexpr coordinate_type left = offset_x;
         static constexpr coordinate_type top  = offset_y;

         static_assert(area > 0, "A lu::fixed_grid cannot be zero-size along either axis.");

         static constexpr size_t no_valid_index = std::numeric_limits<size_t>::max();

      protected:
         list_type _data = {};

      public:
         constexpr const value_type* at(coordinate_type x, coordinate_type y) const;
         constexpr value_type* at(coordinate_type x, coordinate_type y);

         // Reset all grid items to default-constructed values.
         constexpr void clear();

         static constexpr bool coordinates_in_bounds(coordinate_type x, coordinate_type y);

      protected:
         template<coordinate_type x, coordinate_type y>
         static constexpr const bool _coordinates_in_bounds = coordinates_in_bounds(x, y);
      public:

         // Returns `no_valid_index` on failure.
         static constexpr size_t coordinates_to_index(coordinate_type x, coordinate_type y);

         template<coordinate_type x, coordinate_type y>
         static constexpr size_t coordinates_to_index() requires (coordinates_in_bounds(x, y)) {
            return (size_t)(x - left) + ((size_t)(y - top) * width);
         }
         // ^ IntelliSense chokes if we move this definition to the INL file; it gets confused about overloads.

         static constexpr bool flat_index_in_bounds(size_t i);

         #pragma region item
            constexpr const value_type& item(coordinate_type x, coordinate_type y) const;
            constexpr value_type& item(coordinate_type x, coordinate_type y);

            template<coordinate_type x, coordinate_type y>
            constexpr const value_type& item() const noexcept requires (coordinates_in_bounds(x, y)) {
               return this->_data[coordinates_to_index<x, y>()];
            }
            // ^ IntelliSense chokes if we move this definition to the INL file; it gets confused about overloads.

            template<coordinate_type x, coordinate_type y>
            constexpr value_type& item() noexcept requires (coordinates_in_bounds(x, y)) {
               return this->_data[coordinates_to_index<x, y>()];
            }
            // ^ IntelliSense chokes if we move this definition to the INL file; it gets confused about overloads.
         #pragma endregion

         #pragma region item_by_flat_index
            // Access an item by its row-major index, with the top-left item being index 0.
            constexpr const value_type& item_by_flat_index(size_t i) const;

            // Access an item by its row-major index, with the top-left item being index 0.
            constexpr value_type& item_by_flat_index(size_t i);

            // Access an item by its row-major index, with the top-left item being index 0.
            template<size_t i>
            constexpr const value_type& item_by_flat_index() const noexcept requires (i < area);

            // Access an item by its row-major index, with the top-left item being index 0.
            template<size_t i>
            constexpr value_type& item_by_flat_index() noexcept requires (i < area);
         #pragma endregion

         constexpr list_type& list() noexcept;
         constexpr const list_type& list() const noexcept;
   };

   template<typename T, size_t width>
   using corner_square_grid = fixed_grid<T, width, width, 0, 0>;

   template<typename T, size_t halfwidth>
   using centered_square_grid = fixed_grid<T, halfwidth * 2, halfwidth * 2, -(signed int)halfwidth, -(signed int)halfwidth>;
}

#include "./fixed_grid.inl"