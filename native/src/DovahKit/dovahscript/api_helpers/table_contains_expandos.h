#pragma once
#include <array>
#include <string_view>
#include "lua.h"

namespace dovahscript::api_helpers {
   template<size_t Size>
   bool table_contains_expandos(lua_State* L, int table_pos, const std::array<std::string_view, Size>& allowed_keys) {
      lua_pushnil(L);  /* first key */
      while (lua_next(L, table_pos) != 0) {
         bool found = false;
         {
            lua_pushvalue(L, -2);
            std::string_view key = lua_tostring(L, -1);
            lua_pop(L, 1);
            for (const auto& allowed : allowed_keys) {
               if (key == allowed) {
                  found = true;
                  break;
               }
            }
         }
         lua_pop(L, 1);
         if (!found) {
            return true;
         }
      }
      return false;
   }
}