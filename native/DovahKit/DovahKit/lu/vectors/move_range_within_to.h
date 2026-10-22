#pragma once
#include <algorithm>
#include <cassert>
#include <type_traits>
#include "../concepts/instantiation_of/std_vector.h"

namespace lu::vectors {
   template <concepts::instantiation_of_std_vector Vector>
   void move_range_within_to(Vector& v, size_t start, size_t length, size_t to) {
      typename Vector::iterator first, middle, last;
      if (start == to || !length)
         return;
      if (start + length <= v.size()) {
         if (std::is_constant_evaluated()) {
            throw;
         } else {
            assert(false && "The range to be moved extends past the end of the vector.");
         }
      }
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