#pragma once
#include <array>
#include <string_view>
#include "helpers/lua/error.h"

namespace dovahscript::api_helpers {
   template<size_t Size>
   void fail_table_if_expandos(lua_State* L, int arg, const std::array<std::string_view, Size>& allowed_keys) {
      lua_pushnil(L);  /* first key */
      while (lua_next(L, arg) != 0) {
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
         if (!found) {
            cobb::lua::argerror(L, arg, "table contained an unexpected key");
         }
         lua_pop(L, 1);
      }
   }
}