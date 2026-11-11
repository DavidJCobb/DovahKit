#pragma once
#include <type_traits>
#include <vector>
#include "./is_managed_vector_type.h"

namespace dovah::form_data::helpers {
   template<typename T>
   concept is_subobject_vector_type = requires {
      typename T::value_type;
      requires std::is_same_v<T, std::vector<typename T::value_type>> || is_managed_vector_type<T>;
   };
}