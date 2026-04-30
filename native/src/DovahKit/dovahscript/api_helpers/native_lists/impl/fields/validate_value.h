#pragma once
namespace dovah::loaded_forms {
   class Form;
}
struct lua_State;

namespace dovahscript::api_helpers::native_lists::impl::fields::validate_value {
   // The trick I'm using for `present`, for other fields, doesn't seem to work for function overloads.

   template<typename Spec>
   concept valid = requires {
      requires requires(lua_State* L, int i, const dovah::loaded_forms::Form& form, const typename Spec::value_stored_type& dst) {
         { Spec::validate_value(L, i, form) };
         { Spec::validate_value(L, i, form, dst) };
      };
   };
}