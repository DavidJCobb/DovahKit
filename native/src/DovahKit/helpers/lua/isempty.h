#pragma once
#include "../../lua.h"

namespace cobb::lua {
   extern bool isempty(lua_State* L, int table_index = -1); // uses lua_next to test for any keys
}
