#pragma once
#include <array>
#include <string_view>
#include <utility>
#include "lua.h"
#include "helpers/lua/for_each_in_pairs.h"

namespace dovahscript::api_helpers {
   //
   // `retval.first`  indicates whether expandos are present.
   // `retval.second` is the first seen expando, if-and-only-if the key was a string or number.
   //
   template<size_t Size>
   std::pair<bool, std::string> table_contains_expandos(lua_State* L, int table_pos, const std::array<std::string_view, Size>& allowed_keys) {
      table_pos = lua_absindex(L, table_pos);
      const bool table_is_userdata = lua_isuserdata(L, table_pos);

      std::pair<bool, std::string> result;
      cobb::lua::for_each_in_pairs(L, table_pos, [&allowed_keys, &result, table_is_userdata](lua_State* L) {
         if (!lua_isstring(L, -2)) {
            result.first = true;
            return false; // break
         }
         if (table_is_userdata) {
            if (lua_iscfunction(L, -1)) {
               //
               // When dealing with Lua class instances, __pairs will iterate member 
               // functions. Ignore them.
               //
               return true; // continue
            }
         }
         lua_pushvalue(L, -2); // copy the key to avoid converting the original to a string in-place
         std::string_view key = lua_tostring(L, -1);
         {
            bool found = false;
            for (const std::string_view& allowed : allowed_keys) {
               if (key == allowed) {
                  found = true;
                  break;
               }
            }
            if (!found) {
               result.first  = true;
               result.second = std::string(key);
               lua_pop(L, 1); // remove copied key
               return false;  // break
            }
         }
         lua_pop(L, 1); // remove copied key
         return true;   // continue
      });
      return result;
   }
}