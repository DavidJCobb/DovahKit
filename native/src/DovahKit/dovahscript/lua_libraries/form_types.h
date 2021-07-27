#pragma once
#include "../../../lua.h"
#include "../../../dovah/core.h"

namespace dovahscript::lua_libraries {
   namespace form_types {
      extern void import(lua_State* L);

      extern dovah::form_type_t pull(lua_State* L, int stack_pos, bool& valid);
      extern void push(lua_State* L, dovah::form_type_t);
   }
}