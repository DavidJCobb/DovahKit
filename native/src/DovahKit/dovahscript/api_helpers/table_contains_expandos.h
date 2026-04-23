#pragma once
#include <array>
#include <string_view>
#include <utility>
#include "lua.h"

namespace dovahscript::api_helpers {
   //
   // `retval.first`  indicates whether expandos are present.
   // `retval.second` is the first seen expando, if-and-only-if the key was a string or number.
   //
   template<size_t Size>
   std::pair<bool, std::string_view> table_contains_expandos(lua_State* L, int table_pos, const std::array<std::string_view, Size>& allowed_keys) {
      table_pos = lua_absindex(L, table_pos);
      lua_pushnil(L);  /* first key */
      while (lua_next(L, table_pos) != 0) {
         if (!lua_isstring(L, -2)) {
            lua_pop(L, 2);
            return { true, {} };
         }
         std::string_view key;
         bool found = false;
         {
            lua_pushvalue(L, -2);
            key = lua_tostring(L, -1);
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
            return { true, key };
         }
      }
      return { false, {} };
   }
}