#pragma once
#include "../../../../Lua/lua.hpp"

namespace editor_script::namespace_setup {
   //
   // Call when a table is at the top of the stack. Defines all (dovah._____) members on 
   // that table.
   //
   extern void dovah(lua_State* L);
}
