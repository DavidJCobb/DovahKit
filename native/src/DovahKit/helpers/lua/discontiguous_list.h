#pragma once
#include "../../lua.h"

namespace cobb::lua::discontiguous_list {
   extern int insert(lua_State* L, int table_pos); // returns index

   extern void remove(lua_State* L, int table_pos, int index);
}