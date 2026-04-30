#pragma once
#include <concepts>
struct lua_State;

namespace dovahscript::api_helpers::native_lists::impl::fields::pull_value {
   template<typename Spec>
   concept present = requires {
      { Spec::pull_value };
   };
   template<typename Spec>
   concept valid = requires {
      requires present<Spec>;
      requires requires(lua_State* L, int i) {
         { Spec::pull_value(L, i) } -> std::same_as<typename Spec::value_working_type>;
      };
   };
}