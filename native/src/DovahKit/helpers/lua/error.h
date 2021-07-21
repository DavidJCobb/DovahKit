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

   [[noreturn]] inline void argerror(lua_State* L, int arg, const char* message) {
      luaL_argerror(L, arg, message);
   }

   // Identical to luaL_argcheck except that it's not a macro, and it relies on our [[noreturn]] 
   // argerror. This means that if you e.g. use this to error on a null pointer, IntelliSense 
   // should then know not to warn you about subsequent pointer access.
   inline void argcheck(lua_State* L, bool cond, int arg, const char* message) {
      if (!cond)
         argerror(L, arg, message);
   }
}
