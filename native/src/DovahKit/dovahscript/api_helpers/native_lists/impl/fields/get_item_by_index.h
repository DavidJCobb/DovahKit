#pragma once
#include <concepts>
#include "lua.h"

namespace dovahscript::api_helpers::native_lists::impl::fields::get_item_by_index {
   template<typename Spec>
   concept present = requires {
      { Spec::get_item_by_index };
   };

   template<typename Spec>
   concept valid = requires {
      requires present<Spec>;
      requires requires(lua_State* L) {
         { Spec::get_item_by_index(L) } -> std::same_as<int>;
      };
   };
         
   template<typename Spec>
   concept defaultable = requires {
      requires !present<Spec>;
      typename Spec::collection_wrapped_type;
      typename Spec::value_stored_type;
      requires requires(const typename Spec::collection_wrapped_type& list, size_t i) {
         { list[i] } -> std::same_as<const typename Spec::value_stored_type&>;
      };
   };
}