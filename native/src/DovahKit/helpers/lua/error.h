#pragma once
#include "../../lua.h"

namespace cobb::lua {
   // Identical to luaL_error except that it's flagged as [[noreturn]], which may potentially 
   // allow for some compiler optimizations.
   [[noreturn]] inline void error(lua_State* L, const char* fmt, ...) {
      va_list argp;
      va_start(argp, fmt);
      luaL_where(L, 1);
      lua_pushvfstring(L, fmt, argp);
      va_end(argp);
      lua_concat(L, 2);
      lua_error(L);
   }
}
