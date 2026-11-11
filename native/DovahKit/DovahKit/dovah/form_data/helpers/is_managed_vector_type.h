#pragma once
#include <type_traits>
namespace dovah {
   template<typename T>
   class managed_vector;
}

namespace dovah::form_data::helpers {
   template<typename T>
   concept is_managed_vector_type = requires {
      typename T::value_type;
      requires std::is_same_v<T, managed_vector<typename T::value_type>>;
   };
}