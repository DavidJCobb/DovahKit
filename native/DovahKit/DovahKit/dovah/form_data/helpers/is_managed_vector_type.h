#pragma once
#include <type_traits>
namespace dovah::form_data::pieces {
   template<typename T>
   class managed_vector;
}

namespace dovah::form_data_helpers {
   template<typename T>
   concept is_managed_vector_type = requires {
      typename T::value_type;
      requires std::is_same_v<T, form_data::pieces::managed_vector<typename T::value_type>>;
   };
}