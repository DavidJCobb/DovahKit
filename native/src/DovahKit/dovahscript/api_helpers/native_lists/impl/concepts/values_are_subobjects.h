#pragma once
#include <type_traits>

namespace dovahscript::api_helpers::native_lists::impl::concepts {
   template<typename Spec>
   concept values_are_subobjects = requires {
      requires !std::is_same_v<typename Spec::value_wrapper_type, void>;
   };
}