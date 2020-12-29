#include "class_killer.h"
#include <cassert>
#include "../../helpers/lua/metamethod_names.h"

namespace {
   using namespace editor_script;

   bool _should_skip_name(lua_State* L, int index) {
      index = lua_absindex(L, index);
      if (lua_isstring(L, index)) {
         auto nk = lua_tostring(L, index);
         if (cobb::lua::is_metamethod_name(nk))
            return true;
         else if (strcmp(nk, "__getters") == 0)
            return true;
         else if (strcmp(nk, "__setters") == 0)
            return true;
         else if (strcmp(nk, "__superclass") == 0)
            return true;
         else if (strcmp(nk, "__name") == 0)
            return true;
      }
      return false;
   }

   luastackchange_t _dead_method(lua_State* L) {
      return luaL_error(L, "cannot call functions on a dead object");
   }
   luastackchange_t _dead_index(lua_State* L) {
      /*
         local function _dead_index(t, k)
            local meta = getmetatable(t)
            if not meta then
               return nil
            end
            if _should_skip_name(k) then
               return nil
            end
            while true do
               local a = rawget(meta, k)
               if a then
                  return a
               end
               --
               -- Traverse up to the next superclass:
               --
               meta = meta.__superclass
               if not meta then
                  return
               end
            end
         end
      */
      lua_settop(L, 2);
      auto index_table = 1;
      auto index_key   = 2;
      if (!lua_getmetatable(L, index_table)) // STACK: - [ t, k, meta ] +
         return 0;
      auto index_meta = 3;
      if (lua_isnoneornil(L, index_meta))
         return 0;
      do {
         //
         // Check for a member:
         //
         lua_pushvalue(L, index_key);  // STACK: - [ t, k, meta, k       ] +
         lua_rawget   (L, index_meta); // STACK: - [ t, k, meta, meta[k] ] +
         if (!lua_isnoneornil(L, -1))
            return 1;
         lua_settop(L, index_meta); // STACK: - [ t, k, meta ] +
         //
         // Traverse up to the next superclass:
         //
         lua_pushstring(L, "__superclass");
         if (lua_rawget(L, index_meta) == LUA_TTABLE) { // STACK: - [ t, k, v, meta, meta.__superclass ] +
            lua_remove(L, index_meta); // STACK: - [ t, k, v, meta.__superclass ] + // meta = meta.__superclass;
         } else {
            break;
         }
      } while (true);
      return 0;
   }
   luastackchange_t _dead_newindex(lua_State* L) {
      return luaL_error(L, "cannot set properties on a dead object (property name was %s)", lua_tostring(L, 2));
   }

   luastackchange_t _create_zombie_class(lua_State* L) {
      /*
         local function _create_zombie_class(meta)
            local store = registry.__dead_classes
            if not store then
               store = registry.__dead_classes = {}
            end
            local last = nil
            while meta do
               if store[meta.__name] then
                  break
               end
               local dead = {}
               local k, v = next(meta)
               while v do
                  if type(v) == "function" and not _should_skip_name(k) then
                     dead[k] = _dead_method
                  end
                  k, v = next(meta)
               end
               dead.__gc       = meta.__gc
               dead.__index    = _dead_index
               dead.__newindex = _dead_newindex
               --
               dead.__name = "zombie<" .. meta.__name .. ">"
               --
               store[meta.__name] = dead
               if last then
                  last.__superclass = dead
               end
               --
               last = dead
               meta = meta.__superclass
            end
         end
      */
      auto index_meta  = 1;
      auto index_store = 2;
      auto index_last  = 3;
      auto index_dead  = 4;
      auto index_nk    = 5;
      auto index_nv    = 6;
      //
      lua_settop(L, 1);
      lua_getfield(L, LUA_REGISTRYINDEX, dead_class_metatable_storage);
      if (lua_isnoneornil(L, index_store)) {
         lua_settop(L, 1);
         lua_createtable(L, 0, 1);
         lua_pushvalue(L, index_store);
         lua_setfield(L, LUA_REGISTRYINDEX, dead_class_metatable_storage);
      }
      //
      lua_pushnil(L); // last
      while (!lua_isnoneornil(L, index_meta)) { // STACK: - [ meta, store, last ] +
         lua_getfield(L, index_meta, "__name");
         lua_rawget(L, index_store); // STACK: - [ meta, store, last, (dead = store[meta.__name]) ] +
         if (!lua_isnoneornil(L, index_dead))
            break;
         lua_settop(L, index_last);
         lua_createtable(L, 0, 0); // dead
         lua_pushnil(L); // nk
         while (lua_next(L, index_meta) != 0) {
            if (lua_type(L, index_nv) == LUA_TFUNCTION && !_should_skip_name(L, index_nk)) {
               lua_pushvalue    (L, index_nk);
               lua_pushcfunction(L, &_dead_method);
               lua_rawset       (L, index_dead);
            }
            lua_settop(L, index_nk);
         }
         lua_getfield(L, index_meta, "__gc");
         lua_setfield(L, index_dead, "__gc");
         lua_pushcfunction(L, &_dead_index);
         lua_setfield     (L, index_dead, "__index");
         lua_pushcfunction(L, &_dead_newindex);
         lua_setfield     (L, index_dead, "__newindex");
         //
         lua_getfield(L, index_meta, "__name");
         lua_pushfstring(L, "zombie<%s>", lua_tostring(L, -1));
         lua_setfield(L, index_dead, "__name");
         lua_pop(L, 1);
         //
         lua_getfield (L, index_meta, "__name");
         lua_pushvalue(L, index_dead);
         lua_rawset   (L, index_store);
         if (!lua_isnoneornil(L, index_last)) {
            lua_pushvalue(L, index_dead);
            lua_setfield (L, index_last, "__superclass");
         }
         //
         lua_copy(L, index_dead, index_last);
         lua_getfield(L, index_meta, "__superclass");
         lua_replace (L, index_meta);
         lua_settop(L, index_last);
      }
      return 0;
   }
}

namespace editor_script {
   luastackchange_t zombify_userdata(lua_State* L) { // call via lua_call, not directly
      /*
         function kill_class_of(ud)
            local meta  = getmetatable(ud)
            if not meta then
               return
            end
            _create_zombie_class(meta)
            setmetatable(ud, registry.__dead_classes[meta.__name])
         end
      */
      //
      auto index_ud    = 1;
      auto index_meta  = 2;
      auto index_store = 3;
      auto index_dead  = 4;
      //
      lua_settop(L, 1);
      lua_getmetatable(L, index_ud); // 2
      if (lua_isnoneornil(L, index_meta))
         return 0;
      lua_pushcfunction(L, &_create_zombie_class); // 3
      lua_pushvalue(L, index_meta);                // 4
      lua_call(L, 1, 0); // STACK: - [ ud, meta ] +
      lua_getfield(L, LUA_REGISTRYINDEX, dead_class_metatable_storage);
      if (lua_isnoneornil(L, index_store))
         return 0;
      lua_getfield(L, index_meta, "__name");
      lua_rawget  (L, index_store);
      assert(!lua_isnoneornil(L, index_dead));
      lua_setmetatable(L, index_ud);
      return 0;
   }
}