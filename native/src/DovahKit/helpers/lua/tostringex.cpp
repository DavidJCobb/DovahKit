#include "tostringex.h"

namespace cobb::lua {
   // Convert the value at (stack_pos) to a string only if it is already a string or number, or 
   // if it has a __tostring metamethod.
   extern bool tostringex(lua_State* L, int stack_pos, std::string& out) {
      out.clear();
      stack_pos = lua_absindex(L, stack_pos);
      if (luaL_callmeta(L, stack_pos, "__tostring")) { // bare strings are the more common case, but strings actually have a metatable (which they all share), so do this first
         bool result = lua_isstring(L, -1);
         if (result)
            out = lua_tostring(L, -1);
         lua_pop(L, 1);
         return result;
      }
      if (lua_isstring(L, stack_pos)) {
         lua_pushvalue(L, stack_pos); // operate on a copy, to ensure that this can be safely used during lua_next
         out = lua_tostring(L, -1);
         lua_pop(L, 1);
         return true;
      }
      return false;
   }
}
