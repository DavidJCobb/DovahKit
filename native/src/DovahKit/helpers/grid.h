#pragma once
#include <array>
#include <cstdint>
#include <limits>

namespace cobb {
   template<typename T, size_t w, size_t h, signed int offset_x = 0, signed int offset_y = 0> class grid {
      public:
         static constexpr size_t width  = w;
         static constexpr size_t height = h;
         static constexpr size_t area   = w * h;

         using value_type      = T;
         using list_type       = std::array<value_type, area>;
         using coordinate_type = int32_t;

         static constexpr coordinate_type left = offset_x;
         static constexpr coordinate_type top  = offset_y;

         static_assert(area > 0, "A cobb::grid cannot be zero-size along either axis.");

         static constexpr size_t no_valid_index = std::numeric_limits<size_t>::max();

      protected:
         list_type _data = {};

      public:
         value_type* at(coordinate_type x, coordinate_type y) {
            auto i = coordinates_to_index(x, y);
            if (i == no_valid_index)
               return nullptr;
            return &this->_data[i];
         }
         const value_type* at(coordinate_type x, coordinate_type y) const {
            auto i = coordinates_to_index(x, y);
            if (i == no_valid_index)
               return nullptr;
            return &this->_data[i];
         }

         void clear() {
            this->_data = {};
         }

         static constexpr bool coordinates_in_bounds(coordinate_type x, coordinate_type y) {
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
         static constexpr bool flat_index_in_bounds(size_t i) {
            return i < area;
         }

         inline static constexpr size_t coordinates_to_index(coordinate_type x, coordinate_type y) { // returns no_valid_index on failure
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
         template<coordinate_type x, coordinate_type y> requires (coordinates_in_bounds(x, y))
         static constexpr size_t coordinates_to_index() {
            return coordinates_to_index(x, y);
         }

         // Look up an entry by coordinates.
         //  - Run-time:     pass coordinates as arguments; no bounds-checking is performed.
         //  - Compile-time: pass coordinates as template parameters; checks performed via constraints.
         #pragma region item
            value_type& item(coordinate_type x, coordinate_type y) {
               return this->_data[coordinates_to_index(x, y)];
            }
            const value_type& item(coordinate_type x, coordinate_type y) const {
               return this->_data[coordinates_to_index(x, y)];
            }

            template<coordinate_type x, coordinate_type y> value_type& item() {
               return this->_data[coordinates_to_index<x, y>()];
            }
            template<coordinate_type x, coordinate_type y> constexpr const value_type& item() const {
               return this->_data[coordinates_to_index<x, y>()];
            }
         #pragma endregion

         // Look up an entry by flat/linear index.
         //  - Run-time:     pass indices as arguments; no bounds-checking is performed.
         //  - Compile-time: pass indices as template parameters; checks performed via constraints.
         #pragma endregion
            constexpr value_type& by_flat_index(size_t i) {
               return this->_data[i];
            }
            constexpr const value_type& by_flat_index(size_t i) const {
               return this->_data[i];
            }

            template<size_t i> requires (i < area) constexpr value_type& by_flat_index() {
               return this->_data[i];
            }
            template<size_t i> requires (i < area) constexpr const value_type& by_flat_index() const {
               return this->_data[i];
            }
         #pragma endregion

         list_type& list() { return this->_data; }
         const list_type& list() const { return this->_data; }
   };

   template<typename T, size_t width> using corner_square_grid = grid<T, width, width, 0, 0>;

   template<typename T, size_t halfwidth> using centered_square_grid = grid<T, halfwidth * 2, halfwidth * 2, -(signed int)halfwidth, -(signed int)halfwidth>;
}