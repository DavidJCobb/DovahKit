#pragma once
#include <string>
#include "../../lua.h"

namespace dovahscript::core {
   extern void specialized_traceback(lua_State* L, const std::string& message, int level);
}