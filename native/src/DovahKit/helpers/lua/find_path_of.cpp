#include "find_path_of.h"

namespace cobb::lua {
   /*
   function find_path_of(t, value, max_depth)
      if max_depth == 0 or type(t) ~= "table" then
         return ""
      end
      local k, v = next(t)
      while k do
         if type(k) ~= "string" then
            goto continue
         end
         if value == v then
            return k
         end
         local nested = _recursive_find_value_in_table(v, value, max_depth - 1)
         if nested ~= "" then
            return k .. "." .. nested
         end
         ::continue::
         k, v = next(t, k)
      end
      return ""
   end
   */
   [[nodiscard]] extern std::string find_path_of(lua_State* L, int table_index, int value_index, int max_depth) {
      table_index = lua_absindex(L, table_index);
      value_index = lua_absindex(L, value_index);
      //
      std::string out;
      if (max_depth == 0 || !lua_istable(L, table_index))
         return out;
      lua_pushnil(L);
      while (lua_next(L, table_index)) {
         if (lua_type(L, -2) != LUA_TSTRING) {
            lua_pop(L, 1);
            continue;
         }
         if (lua_rawequal(L, value_index, -1)) {
            out = lua_tostring(L, -2);
            lua_pop(L, 2);
            return out;
         }
         auto nested = find_path_of(L, table_index, value_index, max_depth - 1);
         if (!nested.empty()) {
            out  = lua_tostring(L, -2);
            out += '.';
            out += nested;
            lua_pop(L, 2);
            return out;
         }
         lua_pop(L, 1); // for lua_next loop
      }
      return out;
   }
}
