#pragma once
#include <vector>

namespace cobb {
   //
   // Move a range of elements within a vector. Assumes that the range is valid.
   // Per: <https://stackoverflow.com/a/7533658>
   //
   template <typename T> void move_range(std::vector<T>& v, size_t start, size_t length, size_t to) {
      typename std::vector<T>::iterator first, middle, last;
      if (start == to || !length)
         return;
      assert(start + length <= v.size() && "The range to be moved extends past the end of the vector.");
      if (start < to) {
         first  = v.begin() + start;
         middle = first + length;
         last   = v.begin() + to + length;
      } else {
         first  = v.begin() + to;
         middle = v.begin() + start;
         last   = middle + length;
      }
      std::rotate(first, middle, last);
   }
}