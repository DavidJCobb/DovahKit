#pragma once
#include <type_traits>
#include "helpers/function_traits.h"

namespace dovahscript::api_helpers::subobject_property_helpers::utils {
   template<typename T>
   concept is_accessor = requires {
      //
      // Signature must be:
      //    stored_type& func(subobject_type&);
      // 
      // Used to access the property (i.e. field) on a subobject instance, for 
      // both reading and writing.
      //
      typename cobb::function_traits<T>::return_type;
      requires cobb::function_traits<T>::arg_count == 1;
      requires std::is_lvalue_reference_v<typename cobb::function_traits<T>::template arg_type<0>>;
      requires std::is_lvalue_reference_v<typename cobb::function_traits<T>::return_type>;
   };
}