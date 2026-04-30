#pragma once
#include <concepts>
struct lua_State;

namespace dovahscript::api_helpers::native_lists::impl::concepts {
   template<typename Spec>
   concept pushes_value_directly = requires {
      typename Spec::value_stored_type;
      requires requires(lua_State* L, const typename Spec::value_stored_type& v) {
         { Spec::push_value(L, v) } -> std::same_as<int>;
      };
   };
}