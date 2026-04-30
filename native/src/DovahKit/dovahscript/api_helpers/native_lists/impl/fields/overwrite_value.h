#pragma once
namespace dovah::loaded_forms {
   class Form;
}
struct lua_State;

namespace dovahscript::api_helpers::native_lists::impl::fields::overwrite_value {
   template<typename Spec>
   concept present = requires {
      { Spec::overwrite_value };
   };
   template<typename Spec>
   concept valid = requires {
      requires present<Spec>;
      typename Spec::value_stored_type;
      requires requires(lua_State* L, int i, typename Spec::value_stored_type& v, dovah::loaded_forms::Form& form) {
         { Spec::overwrite_value(L, i, v, form) };
      };
   };
}