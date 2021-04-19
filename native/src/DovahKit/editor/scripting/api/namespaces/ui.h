#pragma once
#include "../../../lua.h"

namespace editor_script::namespace_setup {
   //
   // Call when a table is at the top of the stack. Defines all (ui._____) members on 
   // that table (except for class/wrapper singletons).
   //
   extern void ui(lua_State* L);
}
