#pragma once
#include "lua.h"
#include "dovahscript/wrapper.h"
#include "../member_function_spec.h"
#include "./preparation_types.h"
#include "./store_value.h"

namespace dovahscript::api_helpers::native_lists::impl {
   template<typename Spec>
   typename void initialize_implicitly_inserted_list_items(
      wrapper& self,
      typename Spec::collection_wrapped_type& list,
      size_t insert_at,
      const preparation_list_type_t<Spec>& preparations
   ) {
      using value_type = typename Spec::value_working_type;

      const size_t size = list.size();
      if constexpr (impl::fields::initialize_value::present<Spec>) {
         const value_type working = Spec::initialize_value();
         for (size_t j = size; j < insert_at; ++j) {
            impl::exec_store_value<Spec>(working, list[j], *self.form);
            if constexpr (impl::fields::prepare_for_insertion::present<Spec>) {
               Spec::apply_preparation(list[j], preparations[j]);
            }
         }
      } else if constexpr (impl::fields::prepare_for_insertion::present<Spec>) {
         for (size_t j = size; j < insert_at; ++j) {
            Spec::apply_preparation(list[j], preparations[j]);
         }
      }
   }
}