#pragma once
#include "dovahscript/core/subsystems/permissions.h"
#include "./member_spec.h"
#include "../alias.h"

namespace dovahscript::api_helpers::fill_params_helpers {
   template<typename Wrapper, const auto& Member>
   int member_setter(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();

      auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
      auto* alias = wrappers::quest_alias::unwrap(self);
      auto* fill  = Wrapper::unwrap(self);
      if (!fill)
         cobb::lua::error(L, "property access on a zombie object");

      Member.validators.complain(*alias, L, 2);

      self.before_edit();
      Member.write(*alias, *fill, L, 2);
      self.after_edit();
      return 0;
   }
}