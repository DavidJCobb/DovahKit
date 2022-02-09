#pragma once
#include <concepts>

namespace cobb {
   template<typename T> concept has_std_size_getter = requires(const T& x) {
      { x.size() } -> std::same_as<size_t>;
   };
}