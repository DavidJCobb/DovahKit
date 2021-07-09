#pragma once
#include <string>
#include "../../lua.h"

namespace cobb::lua {
   // Convert the value at (stack_pos) to a string only if it is already a string or number, or 
   // if it has a __tostring metamethod.
   extern bool tostringex(lua_State* L, int stack_pos, std::string& out);
}
