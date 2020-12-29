#include "collections.h"
#include <cstdint>

#include "../../../helpers/lua/dump.h"

namespace {
   using namespace editor_script;
   //
   constexpr char* iterator_mt_pairs  = "-dovah-collection-helpers:pairs-iterator";
   constexpr char* iterator_mt_ipairs = "-dovah-collection-helpers:ipairs-iterator";

   void _stringify_and_push_number(lua_State* L, int index) {
      lua_Number f = lua_tonumber(L, index);
      if (f != int64_t(f)) {
         lua_pushnumber(L, f);
      } else {
         lua_pushinteger(L, f);
      }
      lua_tolstring(L, -1, nullptr);
   }

   #pragma region Collection iterator metamethods
      luastackchange_t __pairs_call(lua_State* L) {
         /*
         function(self, t, k)
            local meta = self.meta
            local list = self.names
            local k, v = next(list, nil)
            while not meta.lookup_item_by_name(t, k) do
               k, v = next(list, k)
            end
            if v ~= nil then
               v = meta.lookup_item_by_name(t, k)
               return k, v
            end
         end
         */
         auto index_self = 1;
         auto index_tbl  = 2; // the collection wrapper
         auto index_key  = 3;
         auto index_meta = 4;
         auto index_list = 5;
         auto index_nk   = 6;
         auto index_nv   = 7;
         lua_getfield(L, index_self, "meta");
         lua_getfield(L, index_self, "names");
         //
         lua_pushvalue(L, index_key); // nk
         while (lua_next(L, index_list) != 0) {
            lua_getfield(L, index_meta, "lookup_item_by_name");
            lua_pushvalue(L, index_tbl);
            lua_pushvalue(L, index_nk);
            lua_call(L, 2, 1);
            if (!lua_isnoneornil(L, -1)) {
               lua_replace(L, index_nv);
               return 2;
            }
            lua_settop(L, index_nk);
         }
         return 0;
      }
      luastackchange_t __ipairs_call(lua_State* L) {
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
         lua_pushnumber(L, 1.0F);
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
   #pragma endregion
   //
   void _define_collection_iterator_metatables(lua_State* L) {
      //
      // Define the collection iterator metatables if they don't yet exist.
      //
      auto start = lua_gettop(L);
      //
      if (luaL_getmetatable(L, iterator_mt_pairs) != LUA_TTABLE) {
         lua_settop(L, start); // when the metatable isn't present, nil is pushed
         luaL_newmetatable(L, iterator_mt_pairs);
         lua_pushcfunction(L, &__pairs_call);
         lua_setfield(L, start + 1, "__call");
         lua_settop(L, start);
      }
      if (luaL_getmetatable(L, iterator_mt_ipairs) != LUA_TTABLE) {
         lua_settop(L, start); // when the metatable isn't present, nil is pushed
         luaL_newmetatable(L, iterator_mt_ipairs);
         lua_pushcfunction(L, &__ipairs_call);
         lua_setfield(L, start + 1, "__call");
         lua_settop(L, start);
      }
   }
   
   #pragma region Collection metamethods
   luastackchange_t __index(lua_State* L) {
      /*
      function(self, k)
         local meta  = getmetatable(self)
         local named = meta.items_are_named
         local kt    = type(k)
         if kt == "number" then
            local v = meta.lookup_item_by_index(self, k)
            if v == nil and named then
               v = meta.lookup_item_by_name(self, k)
            end
            return v
         end
         if named then
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
            if named then
               v = meta.lookup_item_by_name(self, k)
            end
            if v == nil and tonumber(k) ~= nil then
               v = meta.lookup_item_by_index(self, k)
            end
            return v
         end
      end
      */
      auto index_self = 1;
      auto index_key  = 2;
      auto index_meta = 3;
      lua_getmetatable(L, index_self);
      //
      lua_getfield(L, index_meta, "items_are_named");
      bool items_are_named = lua_toboolean(L, 4);
      lua_settop(L, index_meta);
      //
      auto type = lua_type(L, index_key);
      if (type == LUA_TNUMBER) {
         lua_getfield (L, index_meta, "lookup_item_by_index");
         lua_pushvalue(L, index_self);
         lua_pushvalue(L, index_key);
         lua_call(L, 2, 1);
         if (items_are_named && lua_isnil(L, 4)) {
            lua_getfield (L, index_meta, "lookup_item_by_name");
            lua_pushvalue(L, index_self);
            _stringify_and_push_number(L, index_key);
            lua_call(L, 2, 1);
         }
         return 1;
      }
      if (items_are_named) {
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
         if (items_are_named) {
            lua_getfield (L, index_meta, "lookup_item_by_name");
            lua_pushvalue(L, index_self);
            lua_pushvalue(L, index_key);
            lua_call(L, 2, 1);
         }
         if (!items_are_named || lua_isnil(L, 4)) {
            if (lua_isnumber(L, index_key)) {
               lua_getfield (L, index_meta, "lookup_item_by_index");
               lua_pushvalue(L, index_self);
               lua_pushvalue(L, index_key);
               lua_call(L, 2, 1);
            } else {
               return 0;
            }
         }
         return 1;
      }
      return 0;
   }
   luastackchange_t __pairs(lua_State* L) {
      /*
      function(self)
         local meta = getmetatable(self)
         if not meta.items_are_named then
            return meta.__ipairs(self)
         end
         --
         local iterator = {}
         iterator.meta  = meta
         iterator.names = meta.get_all_item_names(self)
         setmetatable(iterator, pairs_iterator_mt)
         return iterator, self, nil
      end
      */
      auto index_self = 1;
      auto index_meta = 2;
      auto index_iter = 3;
      lua_getmetatable(L, index_self);
      lua_getfield(L, index_meta, "items_are_named");
      if (!lua_toboolean(L, -1)) {
         auto prior = lua_gettop(L);
         lua_getfield(L, index_meta, "__ipairs");
         lua_pushvalue(L, index_self);
         lua_call(L, 1, LUA_MULTRET);
         return lua_gettop(L) - prior;
      }
      lua_settop(L, index_meta);
      //
      lua_createtable(L, 0, 2);            // push 1 (iter)
      lua_pushvalue(L, index_meta);        // push 1
      lua_setfield(L, index_iter, "meta"); // pop  1
      lua_getfield(L, index_meta, "get_all_item_names"); // push 1
      lua_pushvalue(L, index_self);             // push 1
      lua_call(L, 1, 1);                        // pop  2; push 1
      lua_setfield(L, index_iter, "names");     // pop  1
      luaL_setmetatable(L, iterator_mt_pairs);  // push 0
      //
      lua_pushvalue(L, index_self);
      lua_pushnil(L);
      //
      return 3; // iter, self, nil
   }
   luastackchange_t __ipairs(lua_State* L) {
      /*
      function(self)
         local iterator = {}
         iterator.meta = getmetatable(self)
         setmetatable(iterator, ipairs_iterator_mt)
         return iterator, self, 0
      end
      */
      auto index_self = 1;
      auto index_iter = 2;
      lua_createtable(L, 0, 1);
      lua_getmetatable(L, index_self);
      lua_setfield(L, index_iter, "meta");
      luaL_setmetatable(L, iterator_mt_ipairs);
      lua_pushvalue(L, index_self);
      lua_pushnumber(L, 0.0);
      return 3;
   }
   #pragma endregion
}

namespace editor_script {
   extern void define_collection_metatable(
      lua_State* L,
      const char* registry_key,
      lua_CFunction garbage_collection,    // __gc metamethod (optional)
      bool items_are_named,
      lua_CFunction get_collection_length, // args: wrapper;        return: number
      lua_CFunction lookup_item_by_name,   // args: wrapper, name;  return: wrapper or nil
      lua_CFunction lookup_item_by_index,  // args: wrapper, index; return: wrapper or nil
      lua_CFunction get_all_item_names     // args: wrapper;        return: table of names
   ) {
      _define_collection_iterator_metatables(L);
      //
      luaL_newmetatable(L, registry_key); // STACK: [newmeta]
      auto index_mt = lua_gettop(L);
      //
      lua_pushcfunction(L, &__index);
      lua_setfield(L, index_mt, "__index");
      lua_pushcfunction(L, &__pairs);
      lua_setfield(L, index_mt, "__pairs");
      lua_pushcfunction(L, &__ipairs);
      lua_setfield(L, index_mt, "__ipairs");
      //
      if (garbage_collection) {
         lua_pushcfunction(L, garbage_collection);
         lua_setfield(L, index_mt, "__gc");
      }
      //
      lua_pushboolean(L, items_are_named);
      lua_setfield(L, index_mt, "items_are_named");
      //
      if (get_collection_length) {
         lua_pushcfunction(L, get_collection_length);
         lua_setfield(L, index_mt, "__len");
      }
      //
      if (lookup_item_by_name)
         lua_pushcfunction(L, lookup_item_by_name);
      else
         lua_pushnil(L);
      lua_setfield(L, index_mt, "lookup_item_by_name");
      //
      if (lookup_item_by_index)
         lua_pushcfunction(L, lookup_item_by_index);
      else
         lua_pushnil(L);
      lua_setfield(L, index_mt, "lookup_item_by_index");
      //
      if (get_all_item_names)
         lua_pushcfunction(L, get_all_item_names);
      else
         lua_pushnil(L);
      lua_setfield(L, index_mt, "get_all_item_names");
      //
      lua_pop(L, 1); // pop metatable
   }
}