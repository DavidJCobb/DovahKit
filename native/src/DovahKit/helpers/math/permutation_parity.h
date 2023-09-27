#pragma once
#include <stdexcept>
#include <type_traits>
#include "../bitset.h"

namespace cobb::math {
   template<typename T>
   constexpr int permutation_parity(const T& permuted_indices) requires (std::tuple_size_v<T> > 0 && std::is_convertible_v<std::tuple_element_t<0, T>, size_t>) {
      constexpr const size_t size = std::tuple_size_v<T>;

      size_t swap_count = 0;

      // https://stackoverflow.com/questions/20702782/efficiently-determine-the-parity-of-a-permutation
      //
      cobb::bitset<size> seen;
      for (size_t i = 0; i < size; ++i) {
         if (seen.test(i))
            continue;

         seen.set(i);
         for (size_t j = permuted_indices[i]; !seen.test(j); j = permuted_indices[j]) {
            seen.set(j);
            ++swap_count;
         }
      }

      if (std::is_constant_evaluated()) {
         if (!seen.all())
            throw std::logic_error("`permuted` is not a list of all indices within the bounds [0, size]");
      } else {
         #if _DEBUG
         if (!seen.all())
            throw std::logic_error("`permuted` is not a list of all indices within the bounds [0, size]");
         #endif
      }

      return (swap_count % 2) ? -1 : 1;
   }
}