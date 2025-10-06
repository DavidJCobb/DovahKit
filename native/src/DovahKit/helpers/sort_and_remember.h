#pragma once
#include <algorithm>
#include <concepts>
#include <vector>

namespace cobb {
   template<typename Container, typename Functor>
      requires requires {
         typename Container::value_type;
         requires requires(Container& list, Functor&& functor, typename Container::value_type& item, size_t size) {
            { list.resize(size) };
            { list[size] } -> std::same_as<typename Container::value_type&>;
            { functor(item, item) } -> std::same_as<bool>;
         };
      }
   [[nodiscard]] std::vector<size_t> sort_and_remember(Container& list, Functor&& functor) {
      const size_t size = list.size();

      std::vector<size_t> indices;
      indices.resize(size);
      for (size_t i = 0; i < size; ++i)
         indices[i] = i;

      std::sort(
         indices.begin(),
         indices.end(),
         [&functor, &list](size_t a, size_t b) {
            return functor(list[a], list[b]);
         }
      );

      Container sorted;
      sorted.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         auto  dst  = indices[i];
         sorted[dst] = std::move(item);
      }
      
      std::swap(list, sorted);
      return indices;
   }
}
