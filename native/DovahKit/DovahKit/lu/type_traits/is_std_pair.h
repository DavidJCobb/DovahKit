#pragma once
#include <type_traits>
#include "../concepts/instantiation_of/std_pair.h"

namespace lu {
   template<typename T>
   struct is_std_pair : std::bool_constant<concepts::instantiation_of_std_pair<T>> {};

   template<typename T>
   static constexpr const bool is_std_pair_v = is_std_pair<T>::value;
}