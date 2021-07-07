#pragma once
#include "../../lua.h"

namespace cobb::lua {
   inline bool istablelike(lua_State* L, int table_index = -1) {
      switch (lua_type(L, table_index)) {
         case LUA_TTABLE:
         case LUA_TUSERDATA:
            return true;
      }
      return false;
   }
   inline bool istablelike(int type) {
      return (type == LUA_TTABLE) || (type == LUA_TUSERDATA);
   }
}
