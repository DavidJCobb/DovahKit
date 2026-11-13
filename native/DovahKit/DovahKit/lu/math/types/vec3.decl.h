#pragma once
#include <type_traits>

namespace lu::math {
   template<typename T> requires std::is_arithmetic_v<T>
   class vec3;
}