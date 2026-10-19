#pragma once
#include <type_traits>
#include "../concepts/instantiation_of/std_vector.h"

namespace lu {
   template<typename T>
   struct is_std_vector : std::bool_constant<concepts::instantiation_of_std_vector<T>> {};

   template<typename T>
   static constexpr const bool is_std_vector_v = is_std_vector<T>::value;
}