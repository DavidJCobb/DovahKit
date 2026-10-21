#pragma once
#include <type_traits>
#include <string>

namespace lu::concepts {
   template<typename T> concept instantiation_of_std_basic_string = requires {
      typename T::allocator_type;
      typename T::traits_type;
      typename T::value_type;
      requires std::is_same_v<T, std::basic_string<typename T::value_type, typename T::traits_type, typename T::allocator_type>>;
   };
}