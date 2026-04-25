#pragma once
#include "helpers/lua/error.h"
#include "lua.h"
#include "dovahscript/wrapper.h"

namespace dovahscript::api_helpers::native_lists::common {
   inline dovah::form_stub* pull_value_as_form(lua_State* L, int argpos) {
      if (lua_isnoneornil(L, argpos))
         return nullptr;
      auto* wrapper = wrapper_from_stack<wrappers::form>(L, argpos);
      if (!wrapper || !wrapper->stub)
         cobb::lua::error(L, "expected a form or nil");
      return wrapper->stub;
   }
}