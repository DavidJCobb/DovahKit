#pragma once
#include "helpers/lua/error.h"
#include "helpers/lua/warning.h"
#include "lua.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "./member_function_spec.h"
#include "./impl/get_list_mutation_target.h"
#include "./impl/report_insertion_past_end.h"
#include "./impl/store_value.h"

namespace dovahscript::api_helpers::native_lists {
   template<typename Spec>
      requires (impl::is_fully_valid_spec<Spec> && impl::pull_value::valid<Spec>)
   int set_item_at_index(lua_State* L) {
      using list_type  = typename Spec::collection_wrapped_type;
      using value_type = typename Spec::value_working_type;

      core::subsystems::permissions::verify_form_write_permissions();
      
      constexpr auto index_self  = 1;
      constexpr auto index_key   = 2;
      constexpr auto index_value = 3;

      // Pull args...
      luaL_argcheck(L, lua_isinteger(L, index_key), index_key, "only integer keys are allowed");
      lua_Integer original_index = lua_tointeger(L, index_key);
      if (original_index < 1)
         cobb::lua::error(L, "indices below 1, such as %d, are not allowed", original_index);
      //
      value_type value = Spec::pull_value(L, index_value);
      
      wrapper& self = Spec::pull_collection(L);
      self.load_form();
      auto [list_ptr, insert_at, subobject_index] = impl::get_list_mutation_target<Spec>(L, self, original_index - 1);
      if (!list_ptr)
         cobb::lua::error(L, "internal error: no underlying list to insert into?");
      auto&  list = *list_ptr;
      size_t size = list.size();
      impl::report_insertion_past_end<Spec>(L, list, insert_at, subobject_index);

      self.before_edit();
      if constexpr (Spec::allow_insertions_past_end) {
         if (insert_at >= size) {
            list.resize(insert_at + 1);
            if constexpr (impl::initialize_value::present<Spec>) {
               typename Spec::value_working_type working = Spec::initialize_value();
               for (size_t j = size; j < insert_at; ++j) {
                  impl::exec_store_value<Spec>(working, list[j], *self.form);
               }
            }
         }
      } else {
         if (insert_at == size)
            list.emplace_back();
      }
      impl::exec_store_value<Spec>(value, list[insert_at], *self.form);
      self.after_edit();
      return 0;
   }
}
