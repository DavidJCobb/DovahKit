#pragma once
#include <cassert>
#include "lua.h"
#include "../member_function_spec.h"
#include "./fields/prepare_for_insertion.h"
#include "./preparation_types.h"

namespace dovahscript::api_helpers::native_lists::impl {
   template<typename Spec>
   typename impl::preparation_list_type_t<Spec> make_insertion_preparations(
      typename Spec::collection_wrapped_type& list,
      size_t insert_at,
      lua_State* L,
      int pos_value
   ) {
      impl::preparation_list_type_t<Spec> preparations;
      if constexpr (impl::fields::prepare_for_insertion::present<Spec>) {
         size_t count_to_insert = 1;
         if constexpr (Spec::allow_insertions_past_end) {
            const size_t size = list.size();
            if (insert_at > size)
               count_to_insert = insert_at - size + 1;
         }
         if (!lua_isnoneornil(L, pos_value)) {
            preparations = Spec::prepare_for_insertion(list, count_to_insert, L, pos_value);
         } else {
            preparations = Spec::prepare_for_insertion(list, count_to_insert, L, {});
         }
         assert(preparations.size() == count_to_insert);
      }
      return preparations;
   }
}