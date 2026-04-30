#include "./metamethods.h"
#include "lua.h"

namespace {
   constexpr const char* iterator_mt_ipairs = "_native_list_ipairs";

   // __call metamethod on iterators produced by __ipairs.
   int __ipairs_call(lua_State* L) {
      /*
         function(self, t, k)
            local meta = self.meta
            k = k + 1
            v = meta.lookup_item_by_index(t, k)
            if v ~= nil then
               return k, v
            end
         end
      */
      auto index_self = 1;
      auto index_tbl  = 2; // the collection wrapper
      auto index_key  = 3;
      auto index_meta = 4;
      //
      lua_pushinteger(L, 1);
      lua_arith(L, LUA_OPADD); // key += 1
      //
      lua_getfield(L, index_self, "meta");
      //
      lua_getfield (L, index_meta, "lookup_item_by_index");
      lua_pushvalue(L, index_tbl);
      lua_pushvalue(L, index_key);
      lua_call(L, 2, 1);
      if (!lua_isnoneornil(L, -1)) {
         lua_pushvalue(L, index_key);
         lua_pushvalue(L, -2);
         return 2;
      }
      return 0;
   }
}

namespace dovahscript::api_helpers::native_lists::impl::metamethods {
   int __index(lua_State* L) {
      /*
         function(self, k)
            local meta        = getmetatable(self)
            local has_members = type(meta.members) == "table"
            local kt          = type(k)
            if kt == "number" then
               local v = meta.lookup_item_by_index(self, k)
               if v == nil and has_members then
                  v = meta.members[k]
               end
               return v
            end
            if has_members then
               if kt == "table" or kt == "userdata" then
                  k  = luaL_callmeta(k, "__tostring")
                  kt = type(k)
                  if kt ~= "string" then
                     return
                  end
               end
            end
            if kt == "string" then
               local v
               if tonumber(k) ~= nil then
                  v = meta.lookup_item_by_index(self, k)
               elseif has_members then
                  v = meta.members[k]
               end
               return v
            end
         end
      */
      auto index_self = 1;
      auto index_key  = 2;
      auto index_meta = 3;
      lua_getmetatable(L, index_self);
      bool has_members = lua_getfield(L, index_meta, "members") == LUA_TTABLE;
      lua_pop(L, 1);
      //
      auto type = lua_type(L, index_key);
      if (type == LUA_TNUMBER) {
         lua_getfield (L, index_meta, "lookup_item_by_index");
         lua_pushvalue(L, index_self);
         lua_pushvalue(L, index_key);
         lua_call(L, 2, 1);
         if (lua_isnil(L, 4) && has_members) {
            lua_getfield(L, index_meta, "members");
            lua_pushvalue(L, index_key);
            lua_rawget(L, -2);
         }
         return 1;
      }
      if (has_members) {
         //
         // If the key is a table or userdata, see if it has a __tostring metamethod 
         // that we can use to convert it to a string.
         //
         if (type == LUA_TTABLE || type == LUA_TUSERDATA) {
            if (luaL_callmeta(L, index_key, "__tostring")) {
               if (!lua_isstring(L, -1))
                  return 0;
               lua_replace(L, index_key);
               type = LUA_TSTRING;
            } else {
               return 0;
            }
         }
      }
      if (type == LUA_TSTRING) {
         if (lua_isnumber(L, index_key)) {
            lua_getfield (L, index_meta, "lookup_item_by_index");
            lua_pushvalue(L, index_self);
            lua_pushvalue(L, index_key);
            lua_call(L, 2, 1);
         } else {
            if (has_members) {
               lua_getfield(L, index_meta, "members");
               lua_pushvalue(L, index_key);
               lua_rawget(L, -2);
               return 1;
            }
            return 0;
         }
         return 1;
      }
      return 0;
   }
   int __newindex(lua_State* L) {
      /*
         function(self, key, value)
            local meta = getmetatable(self)
            if not meta.set_item then
               error("you cannot overwrite items in collections of this type")
            end
            if not tonumber(key) then
               error("collections of this type do not support named elements")
            end
            (meta.set_item)(self, key, value)
         end
      */
      constexpr const int index_self  = 1;
      constexpr const int index_key   = 2;
      constexpr const int index_value = 3;
      constexpr const int index_meta  = 4;
      constexpr const int index_func  = 5;
      lua_settop(L, 3);
      lua_getmetatable(L, index_self);
      lua_getfield(L, index_meta, "set_item");
      if (!lua_isfunction(L, index_func)) {
         lua_getfield(L, index_meta, "__name");
         const char* name = lua_tostring(L, 6);
         if (!name)
            name = "?";
         return luaL_error(L, "you cannot overwrite elements in collections of type %1", name);
      }
      if (!lua_isinteger(L, index_key)) {
         lua_getfield(L, index_meta, "__name");
         const char* name = lua_tostring(L, 7);
         if (!name)
            name = "?";
         return luaL_error(L, "collections of type %1 do not support non-integer keys", name);
      }
      lua_settop(L, index_func);
      //
      // STACK:
      // 1 | self
      // 2 | key
      // 3 | value
      // 4 | meta
      // 5 | func
      //
      lua_rotate(L, 1, 1);
      //
      // STACK:
      // 1 | func
      // 2 | self
      // 3 | key
      // 4 | value
      // 5 | meta
      //
      lua_settop(L, 4);
      lua_call(L, 3, 0);
      return 0;
   }
   int __ipairs(lua_State* L) {
      /*
         function(self)
            local iterator = {}
            iterator.meta = getmetatable(self)
            setmetatable(iterator, ipairs_iterator_mt)
            return iterator, self, 0
         end
      */
      constexpr const int index_self = 1;
      lua_createtable(L, 0, 1);
      lua_getmetatable(L, index_self);
      lua_setfield(L, -2, "meta");
      luaL_setmetatable(L, iterator_mt_ipairs);
      lua_pushvalue(L, 1);
      lua_pushinteger(L, 0);
      return 3;
   }

   extern void prepare_iterator_metatables(lua_State* L) {
      auto start = lua_gettop(L);
      if (luaL_getmetatable(L, iterator_mt_ipairs) != LUA_TTABLE) {
         lua_settop(L, start); // when the metatable isn't present, nil is pushed
         luaL_newmetatable(L, iterator_mt_ipairs);
         lua_pushcfunction(L, &__ipairs_call);
         lua_setfield(L, start + 1, "__call");
      }
      lua_settop(L, start);
   }
}