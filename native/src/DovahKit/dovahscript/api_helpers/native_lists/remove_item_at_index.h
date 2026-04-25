#pragma once
#include "lua.h"
#include "dovah/form_reference_t.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "./member_function_spec.h"
#include "./impl/get_list_mutation_target.h"

namespace dovahscript::api_helpers::native_lists {
   template<typename Spec>
      requires (impl::is_fully_valid_spec<Spec> && Spec::allow_removals)
   int remove_item_at_index(lua_State* L) {
      using list_type = typename Spec::collection_wrapped_type;

      core::subsystems::permissions::verify_form_write_permissions();

      luaL_argcheck(L, lua_isinteger(L, 2), 2, "expected an integer index");
      auto original_index = lua_tointeger(L, 2);
      luaL_argcheck(L, original_index >= 1, 2, "index cannot be zero or negative");

      auto& self = Spec::pull_collection(L);
      self.load_form();
      auto [list_ptr, remove_at, _] = impl::get_list_mutation_target<Spec>(L, self, original_index - 1);
      if (!list_ptr)
         return 0;
      auto& list = *list_ptr;
      if (remove_at >= list.size())
         return 0;

      self.before_edit();
      if constexpr (std::is_base_of_v<dovah::form_reference_t, typename Spec::value_stored_type>) {
         list[remove_at].set(*self.form, nullptr);
      }
      list.erase(list.begin() + remove_at);
      self.after_edit();
      return 0;
   }
}
