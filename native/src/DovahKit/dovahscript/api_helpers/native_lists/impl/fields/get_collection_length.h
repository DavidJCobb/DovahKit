#pragma once
#include <concepts>
#include "lua.h"

namespace dovahscript::api_helpers::native_lists::impl::fields::get_collection_length {
   template<typename Spec>
   concept present = requires {
      { Spec::get_collection_length };
   };

   template<typename Spec>
   concept valid = requires {
      requires present<Spec>;
      requires requires(lua_State* L) {
         { Spec::get_collection_length(L) } -> std::same_as<int>;
      };
   };
         
   template<typename Spec>
   concept defaultable = requires {
      requires !present<Spec>;
      typename Spec::collection_wrapped_type;
      requires requires(const typename Spec::collection_wrapped_type& list) {
         { list.size() } -> std::convertible_to<lua_Integer>;
      };
   };
}