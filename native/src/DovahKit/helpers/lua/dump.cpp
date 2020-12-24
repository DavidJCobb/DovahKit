#include "dump.h"
#include "../../../Lua/lua.hpp"
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include "../strings.h"

namespace {
   using table_pointer_vector = std::vector<const void*>;
   std::string _print_value(lua_State* L, table_pointer_vector& seen, int pos, int indent = 0) {
      std::string line = luaL_tolstring(L, pos, nullptr);
      lua_pop(L, 1);
      if (!lua_istable(L, pos))
         return line;
      //
      const void* t  = lua_topointer(L, pos);
      const auto  it = std::find(seen.begin(), seen.end(), t);
      if (it == seen.end()) {
         seen.push_back(t);
         //
         cobb::sprintf(line, "%s = #%i{\n", line.c_str(), seen.size());
         //
         auto prior = lua_gettop(L);
         lua_pushnil(L);
         while (lua_next(L, pos) != 0) {
            std::string field(indent + 3, ' ');
            field += luaL_tolstring(L, -2, nullptr);
            lua_pop(L, 1);
            field += " = ";
            if (lua_type(L, -1) == LUA_TTABLE) {
               field += _print_value(L, seen, lua_absindex(L, -1), indent + 3);
            } else {
               field += luaL_tolstring(L, -1, nullptr);
               lua_pop(L, 1);
            }
            lua_pop(L, 1);
            field += '\n';
            //
            line += field;
         }
         line.reserve(line.size() + indent + 1);
         for (int i = 0; i < indent; ++i)
            line += ' ';
         line += '}';
      } else {
         cobb::sprintf(line, "%s = #%i", line.c_str(), std::distance(seen.begin(), it));
      }
      return line;
   }
}
namespace cobb::lua {
   extern void print_stack(lua_State* L, int stack_start, int stack_end) {
      std::string out;
      if (stack_end < 0)
         stack_end = lua_absindex(L, stack_end);
      auto top = lua_gettop(L);
      if (stack_end > top)
         stack_end = top;
      for (int i = stack_start; i <= stack_end; ++i) {
         std::string line = luaL_tolstring(L, i, nullptr);
         lua_settop(L, top);
         cobb::sprintf(line, "%i: %s", i, line.c_str());
         if (!out.empty())
            out += '\n';
         out += line;
      }
      lua_getglobal(L, "print");
      lua_pushstring(L, out.c_str());
      lua_call(L, 1, 0);
   }
   extern void print_stack_and_vars(lua_State* L, int stack_start, int stack_end) {
      table_pointer_vector seen;
      //
      std::string out;
      if (stack_end < 0)
         stack_end = lua_absindex(L, stack_end);
      auto top = lua_gettop(L);
      if (stack_end > top)
         stack_end = top;
      for (int i = stack_start; i <= stack_end; ++i) {
         std::string line = _print_value(L, seen, i);
         //
         cobb::sprintf(line, "%i: %s", i, line.c_str());
         if (!out.empty())
            out += '\n';
         out += line;
      }
      lua_getglobal(L, "print");
      lua_pushstring(L, out.c_str());
      lua_call(L, 1, 0);
   }
   extern std::string var_to_string(lua_State* L, int pos) {
      table_pointer_vector seen;
      return _print_value(L, seen, pos);
   }
}