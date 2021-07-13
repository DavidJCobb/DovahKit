#pragma once
#include "../../lua.h"

namespace cobb::lua {
   void warning(lua_State* L, const char* fmt, ...);
}
