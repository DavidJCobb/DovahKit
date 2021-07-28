#pragma once
#include "../lua.h"

namespace dovahscript {
   extern int safe_call(lua_State* L, int arg_count, int return_count);
}