#pragma once
#include <concepts>
#include <tuple>

namespace lu::concepts {
   template<typename T>
   concept supports_std_tuple_size = requires {
      { std::tuple_size_v<T> } -> std::same_as<const size_t&>;
   };
}
