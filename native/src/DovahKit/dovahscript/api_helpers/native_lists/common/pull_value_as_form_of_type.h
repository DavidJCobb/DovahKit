#pragma once
#include "helpers/lua/error.h"
#include "lua.h"
#include "dovah/form_types.h"
#include "dovahscript/pull_native_object.h"

namespace dovahscript::api_helpers::native_lists::common {
   template<dovah::form_type Allowed>
   dovah::form_stub* pull_value_as_form_of_type(lua_State* L, int argpos) {
      if (lua_isnoneornil(L, argpos))
         return nullptr;
      auto* stub = pull_form_stub_argument(L, argpos, Allowed);
      if (!stub)
         cobb::lua::error(L, "expected a form or nil");
      return stub;
   }
}