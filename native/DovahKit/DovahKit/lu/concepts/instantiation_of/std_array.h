#pragma once
#include <array>
#include <type_traits>
#include <utility>

namespace lu::concepts {
   template<typename T> concept instantiation_of_std_array = requires {
      typename T::value_type;
      requires !std::is_array_v<T>;
      requires std::is_same_v<T, std::array<typename T::value_type, std::tuple_size_v<T>>>;
   };
}