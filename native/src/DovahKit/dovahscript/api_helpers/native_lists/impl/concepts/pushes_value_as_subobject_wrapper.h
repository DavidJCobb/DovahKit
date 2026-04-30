#pragma once
#include <concepts>
namespace dovahscript {
   class wrapper;
}
struct lua_State;

namespace dovahscript::api_helpers::native_lists::impl::concepts {
   template<typename Spec>
   concept pushes_value_as_subobject_wrapper = requires {
      requires requires(lua_State* L, wrapper& collection, size_t zero_based_item_index) {
         { Spec::push_value(L, collection, zero_based_item_index) } -> std::same_as<int>;
      };
   };
}