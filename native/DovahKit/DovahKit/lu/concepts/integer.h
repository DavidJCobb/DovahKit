#pragma once
#include <type_traits>

namespace lu::concepts {
   template<typename T>
   concept integer = (std::is_integral_v<T> && !std::is_same_v<T, bool>);
}
