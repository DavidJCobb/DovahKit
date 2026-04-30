#pragma once
#include <cassert>
#include <optional>
#include "helpers/lua/error.h"
#include "lua.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "./member_function_spec.h"
#include "./impl/concepts/is_fully_valid_spec.h"
#include "./impl/concepts/is_validate_and_overwrite.h"
#include "./impl/concepts/pushes_value_as_subobject_wrapper.h"
#include "./impl/get_list_mutation_target.h"
#include "./impl/initialize_implicitly_inserted_list_items.h"
#include "./impl/make_insertion_preparations.h"
#include "./impl/preparation_types.h"
#include "./impl/report_insertion_past_end.h"
#include "./impl/store_value.h"

namespace dovahscript::api_helpers::native_lists {
   namespace impl::insert {
      template<typename Spec>
      std::pair<int, std::optional<size_t>> _get_value_pos_and_requested_index(lua_State* L) {
         if constexpr (!std::is_convertible_v<typename Spec::value_stored_type, lua_Integer>) {
            //
            // For collections of numeric values, disallow `coll:insert(pos)` and `coll:insert(pos, value)` 
            // to avoid ambiguity.
            //
            if (lua_isinteger(L, 2)) {
               auto i = lua_tointeger(L, 2);
               cobb::lua::argcheck(L, i > 0, 2, "indices below 1 are not allowed");
               return { 3, (size_t)(i - 1) };
            }
         }
         return { 2, {} };
      }

      template<typename Spec>
      void _do_list_insertion(
         wrapper& self,
         typename Spec::collection_wrapped_type& list,
         size_t insert_at,
         const impl::preparation_list_type_t<Spec>& preparations
      ) {
         if constexpr (Spec::allow_insertions_past_end) {
            if (insert_at > list.size()) {
               list.resize(insert_at + 1);
               impl::initialize_implicitly_inserted_list_items<Spec>(self, list, insert_at, preparations);
            } else {
               list.emplace(list.begin() + insert_at);
            }
         } else {
            list.emplace(list.begin() + insert_at);
         }
      }

      template<typename Spec>
      void _update_forward_siblings(wrapper& self, size_t subobject_index, std::optional<size_t>& requested_index) {
         if constexpr (impl::concepts::values_are_subobjects<Spec>) {
            if (requested_index.has_value()) { // Currently, this function only needs to be called when inserting before the end, hence this check.
               core::subsystems::userdata::get().insert_into_sequential_collection(self, subobject_index);
            }
         }
      }

      // ----------------------------------------------------------------------------------------

      template<typename Spec>
         requires concepts::is_fully_valid_spec<Spec>
      int by_value(lua_State* L) {
         using list_type  = typename Spec::collection_wrapped_type;
         using value_type = typename Spec::value_working_type;

         core::subsystems::permissions::verify_form_write_permissions();

         auto [pos_value, requested_index] = _get_value_pos_and_requested_index<Spec>(L);
         auto& self = Spec::pull_collection(L);
         self.load_form();
         auto [list_ptr, insert_at, subobject_index] = get_list_mutation_target<Spec>(L, self, requested_index);

         std::optional<value_type> value_to_insert;
         if constexpr (fields::pull_value::present<Spec>) {
            if (!lua_isnoneornil(L, pos_value)) {
               value_to_insert = Spec::pull_value(L, pos_value);
            }
         }

         if (!list_ptr)
            cobb::lua::error(L, "internal error: no underlying list to insert into?");
         auto& list = *list_ptr;
         auto  size = list.size();
         impl::report_insertion_past_end<Spec>(L, list, insert_at, subobject_index);

         const auto preparations = make_insertion_preparations<Spec>(list, insert_at, L, pos_value);

         self.before_edit();
         {
            // Insert a new element. If insertion results in the incidental creation of other 
            // elements, thne initialize them and apply preparations as necessary.
            _do_list_insertion<Spec>(self, list, insert_at, preparations);

            // Initialize/overwrite the inserted value, and apply preparations, if either are 
            // necessary.
            if (value_to_insert.has_value()) {
               exec_store_value<Spec>(value_to_insert.value(), list[insert_at], *self.form);
            } else {
               if constexpr (fields::initialize_value::present<Spec>) {
                  const value_type working = Spec::initialize_value();
                  exec_store_value<Spec>(working, list[insert_at], *self.form);
               }
            }
            if constexpr (fields::prepare_for_insertion::present<Spec>) {
               Spec::apply_preparation(list[insert_at], preparations.back());
            }

            // If this is a collection of sub-objects, and we're inserting into the middle, 
            // then update any extant wrappers for the (now displaced) next-siblings.
            _update_forward_siblings<Spec>(self, subobject_index, requested_index);
         }
         self.after_edit();

         if constexpr (concepts::values_are_subobjects<Spec>) {
            static_assert(concepts::pushes_value_as_subobject_wrapper<Spec>);
            return Spec::push_value(L, self, subobject_index);
         } else {
            return 0;
         }
      }

      template<typename Spec>
         requires (concepts::is_fully_valid_spec<Spec> && concepts::is_validate_and_overwrite<Spec>)
      int by_overwrite(lua_State* L) {
         using list_type  = typename Spec::collection_wrapped_type;

         core::subsystems::permissions::verify_form_write_permissions();

         auto [pos_value, requested_index] = _get_value_pos_and_requested_index<Spec>(L);
         auto& self = Spec::pull_collection(L);
         self.load_form();
         auto [list_ptr, insert_at, subobject_index] = get_list_mutation_target<Spec>(L, self, requested_index);

         bool has_value = false;
         if (!lua_isnoneornil(L, pos_value)) {
            has_value = true;
            Spec::validate_value(L, pos_value, *self.form);
         }

         if (!list_ptr)
            cobb::lua::error(L, "internal error: no underlying list to insert into?");
         auto& list = *list_ptr;
         auto  size = list.size();
         report_insertion_past_end<Spec>(L, list, insert_at, subobject_index);

         const auto preparations = make_insertion_preparations<Spec>(list, insert_at, L, pos_value);

         self.before_edit();
         {
            // Insert a new element. If insertion results in the incidental creation of other 
            // elements, thne initialize them and apply preparations as necessary.
            _do_list_insertion<Spec>(self, list, insert_at, preparations);

            // Initialize/overwrite the inserted value, and apply preparations, if either are 
            // necessary.
            if (has_value) {
               Spec::overwrite_value(L, pos_value, list[insert_at], *self.form);
            } else {
               if constexpr (fields::initialize_value::present<Spec>) {
                  const auto working = Spec::initialize_value();
                  impl::exec_store_value<Spec>(working, list[insert_at], *self.form);
               }
            }
            if constexpr (fields::prepare_for_insertion::present<Spec>) {
               Spec::apply_preparation(list[insert_at], preparations.back());
            }

            // If this is a collection of sub-objects, and we're inserting into the middle, 
            // then update any extant wrappers for the (now displaced) next-siblings.
            _update_forward_siblings<Spec>(self, subobject_index, requested_index);
         }
         self.after_edit();

         if constexpr (concepts::values_are_subobjects<Spec>) {
            static_assert(concepts::pushes_value_as_subobject_wrapper<Spec>);
            return Spec::push_value(L, self, subobject_index);
         } else {
            return 0;
         }
      }
   }

   template<typename Spec>
      requires impl::concepts::is_fully_valid_spec<Spec>
   int insert(lua_State* L) {
      if constexpr (impl::concepts::is_validate_and_overwrite<Spec>) {
         return impl::insert::by_overwrite<Spec>(L);
      } else {
         return impl::insert::by_value<Spec>(L);
      }
   }
}
