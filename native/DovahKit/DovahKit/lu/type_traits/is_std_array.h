#pragma once
#include <type_traits>
#include "../concepts/instantiation_of/std_array.h"

namespace lu {
   template<typename T>
   struct is_std_array : std::bool_constant<concepts::instantiation_of_std_array<T>> {};

   template<typename T>
   static constexpr const bool is_std_array_v = is_std_array<T>::value;
}