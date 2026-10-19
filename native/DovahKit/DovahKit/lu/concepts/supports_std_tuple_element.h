#pragma once
#include <tuple>

namespace lu::concepts {
   template<typename T>
   concept supports_std_tuple_element = requires {
      requires (std::tuple_size_v<T> > 0);
      typename std::tuple_element<0, T>::type; // MSVC STL uses `static_assert` to enforce in-bounds, so we can't blindly check even in a concept
   };
}
