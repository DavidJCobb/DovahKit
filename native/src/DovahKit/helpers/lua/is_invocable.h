#pragma once
#include "../../lua.h"

namespace cobb::lua {
   inline bool is_invocable(lua_State* L, int index = -1) {
      index = lua_absindex(L, index);
      switch (lua_type(L, index)) {
         case LUA_TFUNCTION:
            return true;
         case LUA_TTABLE:
         case LUA_TUSERDATA:
            break;
         default:
            return false;
      }
      if (lua_getmetatable(L, index) == 0)
         return false;
      int t = lua_getfield(L, -1, "__call");
      lua_pop(L, 1);
      return t == LUA_TFUNCTION;
   }
}
