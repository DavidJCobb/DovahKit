#pragma once
#include "isempty.h"

namespace cobb::lua {
   extern bool isempty(lua_State* L, int table_index) {
      table_index = lua_absindex(L, table_index);
      auto pos = lua_gettop(L);
      lua_pushnil(L);  // key; popped by next call
      if (lua_next(L, table_index) != 0) {
         lua_settop(L, pos); // two pushes if next was non-zero
         return true;
      }
      return false;
   }
}
