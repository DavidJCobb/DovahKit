#pragma once
#include "../../../Lua/lua.hpp"

namespace editor_script {
   extern void prune_standard_library(lua_State*, const char* libname); // assumes the library is at the top of the stack, having just been loaded with luaL_requiref. pops the library table from the stack.
}