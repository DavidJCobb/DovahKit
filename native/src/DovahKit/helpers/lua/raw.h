#pragma once
#include "../../lua.h"

namespace cobb::lua {
   inline int rawgetfield(lua_State* L, int table_pos, const char* field) noexcept {
      table_pos = lua_absindex(L, table_pos);
      lua_pushstring(L, field);
      return lua_rawget(L, table_pos);
   }

   // Given a value at the top of the stack, write it to table_pos[field].
   inline void rawsetfield(lua_State* L, int table_pos, const char* field) noexcept {
      table_pos = lua_absindex(L, table_pos);
      lua_pushstring(L, field);
      lua_rotate(L, -2, 1);
      lua_rawset(L, table_pos);
   }

   inline int rawgetvalue(lua_State* L, int table_pos, int key_pos) noexcept {
      table_pos = lua_absindex(L, table_pos);
      lua_pushvalue(L, key_pos);
      return lua_rawget(L, table_pos);
   }
}