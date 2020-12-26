#include "setfuncs.h"

namespace cobb::lua {
   void setfuncs(lua_State* L, const std::initializer_list<luaL_Reg>& list) {
      auto top = lua_gettop(L);
      for (const auto& entry : list) {
         if (!entry.name)
            continue;
         if (!entry.func) { // placeholder
            lua_pushboolean(L, 0);
            lua_setfield(L, top, entry.name);
            continue;
         }
         lua_pushcfunction(L, entry.func);
         lua_setfield(L, top, entry.name);
      }
   }
   void setfuncs(lua_State* L, const std::initializer_list<luaL_Reg>& list, int upvalues) {
      auto table = lua_gettop(L) - upvalues;
      for (const auto& entry : list) {
         if (!entry.name)
            continue;
         if (!entry.func) { // placeholder
            lua_pushboolean(L, 0);
            lua_setfield(L, table, entry.name);
            continue;
         }
         for (int i = 0; i < upvalues; ++i)
            lua_pushvalue(L, -upvalues);
         lua_pushcfunction(L, entry.func);
         lua_setfield(L, table, entry.name);
      }
      lua_pop(L, upvalues);
   }
}