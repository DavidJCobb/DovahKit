#pragma once
#include <algorithm>
#include <array>
#include <type_traits>

namespace cobb::arrays {
   namespace impl {
      template<typename T, size_t N, typename Compare>
      constexpr void _quicksort_ct(std::array<T, N>& src, Compare& compare, size_t low, size_t high) {
         if (high <= low)
            return;
         size_t i   = low;
         size_t j   = high + 1;
         auto   key = src[low];
         while (true) {
            while (compare(src[++i], key))
               if (i == high)
                  break;
            while (compare(key, src[--j]))
               if (j == low)
                  break;
            if (i >= j)
               break;

            std::swap(src[i], src[j]);
         }
         std::swap(src[low], src[j]);

         if (j > low + 1)
            _quicksort_ct(src, compare, low, j - 1);
         if (j + 1 < high)
            _quicksort_ct(src, compare, j + 1, high);
      }
   }

   template<typename T, size_t N>
   constexpr void quicksort_ct(std::array<T, N>& src) {
      auto compare = std::less<T>{};
      impl::_quicksort_ct(src, compare, 0, N - 1);
   }

   template<typename T, size_t N, typename Compare>
   constexpr void quicksort_ct(std::array<T, N>& src, Compare compare) {
      impl::_quicksort_ct(src, compare, 0, N - 1);
   }
}