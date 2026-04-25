#pragma once
#include <type_traits>
#include "lua.h"

namespace cobb::lua {
   template<typename Functor>
   void for_each_in_pairs(lua_State* L, int table_pos, Functor&& functor) {
      // STACK: [..., table]
      table_pos = lua_absindex(L, table_pos);
      auto table_type = lua_type(L, table_pos);
      if (table_type == LUA_TUSERDATA) {
         //
         // The `lua_next` function doesn't appear to work on userdata, but also 
         // doesn't appear to properly guard against being invoked on one. We 
         // need to recreate the behavior of Lua for loops and `pairs`: check for 
         // `__pairs` and call it to get an iterator, and then manually invoke 
         // that iterator in a loop.
         //
         if (!luaL_getmetafield(L, table_pos, "__pairs")) {
            lua_pop(L, 1);
            return;
         }
         lua_pushvalue(L, table_pos); // STACK: [table, __pairs, ..., table]
         lua_call(L, 1, 3); // table:__pairs() // STACK: [nil, table, next_func, ..., table]
         if (lua_isnil(L, -3)) {
            lua_pop(L, 3);
            return;
         }
         lua_pop(L, 2); // STACK: [next_func, ..., table]
         int next_func_pos = lua_gettop(L);
         lua_pushnil(L);
         do {
            lua_pushvalue(L, next_func_pos);      // STACK: [next_func, last_key, next_func, ..., table]
            lua_pushvalue(L, table_pos);          // STACK: [table, next_func, last_key, next_func, ..., table]
            lua_rotate(L, next_func_pos + 1, -1); // STACK: [last_key, table, next_func, next_func, ..., table]
            lua_call(L, 2, 2);                    // STACK: [this_value, this_key, next_func, ..., table]
            if (lua_isnil(L, -1)) {
               lua_pop(L, 2); // STACK: [next_func, ..., table]
               break;
            }
            if constexpr (std::is_invocable_r_v<Functor, bool, lua_State*>) {
               bool result = functor(L);
               if (!result) {
                  lua_pop(L, 2); // STACK: [next_func, ..., table]
                  break;
               }
            } else {
               functor(L);
            }
            lua_pop(L, 1); // STACK: [this_key, next_func, ..., table]
         } while (true);
         lua_pop(L, 1); // STACK: [..., table]
      } else if (table_type == LUA_TTABLE) {
         lua_pushnil(L);
         while (lua_next(L, table_pos) != 0) {
            if constexpr (std::is_invocable_r_v<Functor, bool, lua_State*>) {
               bool result = functor(L);
               if (result) {
                  lua_pop(L, 1);
               } else {
                  lua_pop(L, 2);
                  return;
               }
            } else {
               functor(L);
               lua_pop(L, 1); // remove value
            }
         }
      }
   }
}
