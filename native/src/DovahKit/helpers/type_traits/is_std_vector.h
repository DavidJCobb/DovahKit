#pragma once
#include <type_traits>
#include <vector>

namespace cobb {
   template<typename T> concept is_std_vector = requires(T& x) {
      typename T::allocator_type;
      typename T::value_type;
      std::is_same_v<T, std::vector<typename T::value_type, typename T::allocator_type>>;
   };
}