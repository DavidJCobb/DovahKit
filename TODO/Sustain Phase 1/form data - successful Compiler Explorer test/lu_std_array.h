#pragma once
#include <array>
#include <tuple>
#include <type_traits>

namespace lu {
   template<typename T>
   concept std_array = requires {
      typename T::value_type;
      typename std::tuple_element_t<0, T>;
      requires std::is_same_v<T, std::array<typename T::value_type, std::tuple_size_v<T>>>;
   };
}