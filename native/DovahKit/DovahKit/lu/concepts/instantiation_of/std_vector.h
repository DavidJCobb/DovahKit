#pragma once
#include <type_traits>
#include <vector>

namespace lu::concepts {
   template<typename T> concept instantiation_of_std_vector = requires {
      typename T::allocator_type;
      typename T::value_type;
      requires std::is_same_v<T, std::vector<typename T::value_type, typename T::allocator_type>>;
   };
}