#pragma once
#include "helpers/lua/error.h"
#include "lua.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "./impl/concepts/is_fully_valid_spec.h"
#include "./impl/concepts/is_validate_and_overwrite.h"
#include "./impl/get_list_mutation_target.h"
#include "./impl/initialize_implicitly_inserted_list_items.h"
#include "./impl/make_insertion_preparations.h"
#include "./impl/report_insertion_past_end.h"
#include "./impl/store_value.h"

namespace dovahscript::api_helpers::native_lists {
   template<typename Spec>
      requires (
         impl::concepts::is_fully_valid_spec<Spec>
      && (impl::fields::pull_value::valid<Spec> || impl::fields::validate_value::valid<Spec>)
      )
   int set_item_at_index(lua_State* L) {
      struct dummy {};
      using value_variable_type = std::conditional_t<
         impl::concepts::is_validate_and_overwrite<Spec>,
         dummy,
         typename Spec::value_working_type
      >;

      core::subsystems::permissions::verify_form_write_permissions();
      
      constexpr auto index_self  = 1;
      constexpr auto index_key   = 2;
      constexpr auto index_value = 3;

      // Pull args...
      luaL_argcheck(L, lua_isinteger(L, index_key), index_key, "only integer keys are allowed");
      lua_Integer original_index = lua_tointeger(L, index_key);
      if (original_index < 1)
         cobb::lua::error(L, "indices below 1, such as %d, are not allowed", original_index);
      
      value_variable_type value;
      if constexpr (!impl::concepts::is_validate_and_overwrite<Spec>) {
         value = Spec::pull_value(L, index_value);
      }

      wrapper& self = Spec::pull_collection(L);
      self.load_form();
      auto [list_ptr, insert_at, subobject_index] = impl::get_list_mutation_target<Spec>(L, self, original_index - 1);
      if (!list_ptr)
         cobb::lua::error(L, "internal error: no underlying list to insert into?");
      auto&  list = *list_ptr;
      size_t size = list.size();
      impl::report_insertion_past_end<Spec>(L, list, insert_at, subobject_index);
      if constexpr (impl::concepts::is_validate_and_overwrite<Spec>) {
         if (insert_at < list.size()) {
            Spec::validate_value(L, index_value, *self.form, list[insert_at]);
         } else {
            Spec::validate_value(L, index_value, *self.form);
         }
      }

      const auto preparations = impl::make_insertion_preparations<Spec>(list, insert_at, L, index_value);

      self.before_edit();
      if constexpr (Spec::allow_insertions_past_end) {
         if (insert_at >= size) {
            list.resize(insert_at + 1);
            impl::initialize_implicitly_inserted_list_items<Spec>(self, list, insert_at, preparations);
         }
      } else {
         if (insert_at == size)
            list.emplace_back();
      }
      if constexpr (impl::concepts::is_validate_and_overwrite<Spec>) {
         Spec::overwrite_value(L, index_value, list[insert_at], *self.form);
      } else {
         impl::exec_store_value<Spec>(value, list[insert_at], *self.form);
      }
      if constexpr (impl::fields::prepare_for_insertion::present<Spec>) {
         Spec::apply_preparation(list[insert_at], preparations.back());
      }
      self.after_edit();
      return 0;
   }
}
