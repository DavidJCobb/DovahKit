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
         list_type _data;

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

         inline static constexpr size_t coordinates_to_index(coordinate_type x, coordinate_type y) { // returns no_valid_index on failure
            if (x < left)
               return no_valid_index;
            if (y < top)
               return no_valid_index;
            x -= left;
            y -= top;
            if (x >= width)
               return no_valid_index;
            if (y >= height)
               return no_valid_index;
            return (size_t)x + ((size_t)y * (size_t)width);
         }
         template<coordinate_type x, coordinate_type y> static consteval size_t coordinates_to_index() { // returns no_valid_index on failure
            return coordinates_to_index(x, y);
         }

         value_type& item(coordinate_type x, coordinate_type y) {
            return this->_data[coordinates_to_index(x, y)];
         }
         const value_type& item(coordinate_type x, coordinate_type y) const {
            return this->_data[coordinates_to_index(x, y)];
         }
         template<coordinate_type x, coordinate_type y> value_type& item() {
            static_assert(coordinates_to_index<x, y>() != no_valid_index, "Coordinates must be in bounds.");
            return this->_data[coordinates_to_index<x, y>()];
         }
         template<coordinate_type x, coordinate_type y> const value_type& item() const {
            static_assert(coordinates_to_index<x, y>() != no_valid_index, "Coordinates must be in bounds.");
            return this->_data[coordinates_to_index<x, y>()];
         }

         list_type& list() { return this->_data; }
         const list_type& list() const { return this->_data; }
   };

   template<typename T, size_t halfwidth> using centered_square_grid = grid<T, halfwidth * 2, halfwidth * 2, -(signed int)halfwidth, -(signed int)halfwidth>;
}