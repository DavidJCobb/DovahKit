#pragma once
#include <array>
#include "../../lua.h"

namespace cobb::lua {
   extern void setfuncs(lua_State* L, const std::initializer_list<luaL_Reg>&);
   extern void setfuncs(lua_State* L, const std::initializer_list<luaL_Reg>&, int upvalues);
}
