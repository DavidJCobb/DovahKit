#pragma once
#include <optional>
#include "helpers/lua/error.h"
#include "helpers/lua/warning.h"
#include "lua.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "./member_function_spec.h"
#include "./impl/get_list_mutation_target.h"
#include "./impl/report_insertion_past_end.h"
#include "./impl/store_value.h"

namespace dovahscript::api_helpers::native_lists {
   template<typename Spec>
      requires impl::is_fully_valid_spec<Spec>
   int insert(lua_State* L) {
      using list_type  = typename Spec::collection_wrapped_type;
      using value_type = typename Spec::value_working_type;

      core::subsystems::permissions::verify_form_write_permissions();

      std::optional<size_t> requested_index;

      int pos_value = 2;
      if constexpr (!std::is_convertible_v<typename Spec::value_stored_type, lua_Integer>) {
         //
         // For collections of numeric values, disallow `coll:insert(pos)` and `coll:insert(pos, value)` 
         // to avoid ambiguity.
         //
         if (lua_isinteger(L, 2)) {
            pos_value       = 3;
            auto i = lua_tointeger(L, 2);
            cobb::lua::argcheck(L, i > 0, 2, "indices below 1 are not allowed");
            requested_index = i - 1;
         }
      }

      auto& self     = Spec::pull_collection(L);
      self.load_form();
      auto [list_ptr, insert_at, subobject_index] = impl::get_list_mutation_target<Spec>(L, self, requested_index);

      std::optional<value_type> value_to_insert;
      if constexpr (impl::pull_value::present<Spec>) {
         if (!lua_isnoneornil(L, pos_value)) {
            value_to_insert = Spec::pull_value(L, pos_value);
         }
      }

      if (!list_ptr)
         cobb::lua::error(L, "internal error: no underlying list to insert into?");
      auto& list = *list_ptr;
      auto  size = list.size();
      impl::report_insertion_past_end<Spec>(L, list, insert_at, subobject_index);

      self.before_edit();
      if constexpr (Spec::allow_insertions_past_end) {
         if (insert_at > size) {
            list.resize(insert_at + 1);
            if constexpr (impl::initialize_value::present<Spec>) {
               const value_type working = Spec::initialize_value();
               for (size_t j = size; j < insert_at; ++j) {
                  impl::exec_store_value<Spec>(working, list[j], *self.form);
               }
            }
         } else {
            list.emplace(list.begin() + insert_at);
         }
      } else {
         list.emplace(list.begin() + insert_at);
      }
      if (value_to_insert.has_value()) {
         impl::exec_store_value<Spec>(value_to_insert.value(), list[insert_at], *self.form);
      } else {
         if constexpr (impl::initialize_value::present<Spec>) {
            const value_type working = Spec::initialize_value();
            impl::exec_store_value<Spec>(working, list[insert_at], *self.form);
         }
      }
      if constexpr (impl::values_are_subobjects<Spec>) {
         if (requested_index.has_value()) { // Currently, this function only needs to be called when inserting before the end, hence this check.
            core::subsystems::userdata::get().insert_into_sequential_collection(self, subobject_index);
         }
      }
      self.after_edit();

      if constexpr (impl::values_are_subobjects<Spec>) {
         static_assert(impl::pushes_value_as_subobject_wrapper<Spec>);
         return Spec::push_value(L, self, subobject_index);
      } else {
         return 0;
      }
   }
}
