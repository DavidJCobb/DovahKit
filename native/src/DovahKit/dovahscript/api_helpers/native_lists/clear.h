#pragma once
#include "dovah/form_reference_t.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "./impl/concepts/is_fully_valid_spec.h"
#include "./impl/get_list_mutation_target.h"
#include "./impl/list_item_needs_form_to_clear.h"
#include "./impl/list_needs_form_to_clear.h"
struct lua_State;

namespace dovahscript::api_helpers::native_lists {
   template<typename Spec>
      requires (impl::concepts::is_fully_valid_spec<Spec> && Spec::allow_removals)
   int clear(lua_State* L) {
      using list_type = typename Spec::collection_wrapped_type;

      core::subsystems::permissions::verify_form_write_permissions();

      auto& self = Spec::pull_collection(L);
      self.load_form();
      auto [list_ptr, _, __] = impl::get_list_mutation_target<Spec>(L, self, 0);
      if (!list_ptr)
         return 0;
      auto& list = *list_ptr;
      if (list.empty())
         return 0;

      self.before_edit();
      if constexpr (impl::list_needs_form_to_clear<Spec>) {
         list.clear(*self.form);
      } else {
         if constexpr (std::is_base_of_v<dovah::form_reference_t, typename Spec::value_stored_type>) {
            for (auto& item : list)
               item.set(*self.form, nullptr);
         } else if constexpr (impl::list_item_needs_form_to_clear<Spec>) {
            for (auto& item : list)
               item.clear(*self.form);
         }
         list.clear();
      }
      self.after_edit();
      return 0;
   }
}
