#pragma once
#include <type_traits>
#include <utility>

namespace lu::concepts {
   template<typename T> concept instantiation_of_std_pair = requires {
      typename T::first_type;
      typename T::second_type;
      requires std::is_same_v<T, std::pair<typename T::first_type, typename T::second_type>>;
   };
}