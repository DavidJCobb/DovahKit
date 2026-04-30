#pragma once
#include "lua.h"
#include "./impl/concepts/is_fully_valid_spec.h"
#include "./impl/concepts/pushes_value_as_subobject_wrapper.h"
#include "./impl/concepts/pushes_value_directly.h"
#include "./impl/get_list_read_target.h"
#include "./impl/push_value.h"

namespace dovahscript::api_helpers::native_lists {
   template<typename Spec>
      requires (impl::concepts::is_fully_valid_spec<Spec> && impl::fields::get_item_by_index::defaultable<Spec>)
   int get_item_by_index(lua_State* L) {
      auto original_index = lua_tointeger(L, 2);
      if (original_index <= 0)
         return 0;

      auto& self = Spec::pull_collection(L);
      auto [list_ptr, stored_index] = impl::get_list_read_target<Spec>(self, original_index - 1);
      if (!list_ptr)
         return 0;
      const auto& list = *list_ptr;
      if (stored_index >= list.size())
         return 0;

      if constexpr (impl::concepts::values_are_subobjects<Spec>) {
         static_assert(impl::concepts::pushes_value_as_subobject_wrapper<Spec>);
         return Spec::push_value(L, self, stored_index);
      } else if constexpr (impl::concepts::pushes_value_directly<Spec>) {
         return Spec::push_value(L, list[stored_index]);
      } else {
         static_assert(impl::value_type_is_default_pushable<Spec>);
         return impl::default_push_value(L, list[stored_index]);
      }
   }
}
