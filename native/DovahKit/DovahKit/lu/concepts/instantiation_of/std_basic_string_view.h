#pragma once
#include <type_traits>
#include <string_view>

namespace lu::concepts {
   template<typename T> concept instantiation_of_std_basic_string_view = requires {
      typename T::traits_type;
      typename T::value_type;
      requires std::is_same_v<T, std::basic_string_view<typename T::value_type, typename T::traits_type>>;
   };
}