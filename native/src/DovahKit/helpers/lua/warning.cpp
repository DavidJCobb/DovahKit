#include "warning.h"

namespace cobb::lua {
   void warning(lua_State* L, const char* fmt, ...) {
      va_list argp;
      va_start(argp, fmt);
      lua_checkstack(L, 2);
      luaL_where(L, 1);
      lua_pushvfstring(L, fmt, argp);
      va_end(argp);
      lua_concat(L, 2);
      auto* str = lua_tostring(L, -1);
      lua_warning(L, str, 0);
      lua_pop(L, 1);
   }
}
