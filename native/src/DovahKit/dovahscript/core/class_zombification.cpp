#include "class_zombification.h"
#include <cassert>
#include <cstdint>
#include "../../helpers/lua/error.h"
#include "../../helpers/lua/metamethod_names.h"
#include "../../helpers/lua/raw.h"

namespace {
   static constexpr const char* zombie_sentinel_key = "dovahscript.internal.zombification.sentinel";
}

namespace {
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
         else if (strcmp(nk, "__superclasses") == 0)
            return true;
         else if (strcmp(nk, "__classlist") == 0)
            return true;
         else if (strcmp(nk, "__name") == 0)
            return true;
         else if (strcmp(nk, "__custom_class_internals") == 0)
            return true;
         else if (strcmp(nk, "__iszombie") == 0) // unique to zombie classes
            return true;
      }
      return false;
   }

   #pragma region Zombie class metamethods and members
   int _dead_method(lua_State* L) {
      cobb::lua::error(L, "cannot call functions on a dead object");
   }
   int _dead_index(lua_State* L) {
      /*
         local function _dead_index(t, k)
            if _should_skip_name(k) then
               return nil
            end
            local meta = getmetatable(t)
            if not meta then
               return nil
            end
            return rawget(meta, k)
         end
      */
      constexpr auto index_table = 1;
      constexpr auto index_key   = 2;
      if (_should_skip_name(L, index_key))
         return 0;
      lua_settop(L, 2);
      if (!lua_getmetatable(L, index_table)) // STACK: - [ t, k, meta ] +
         return 0;
      constexpr auto index_meta = 3;
      if (lua_isnoneornil(L, index_meta))
         return 0;
      lua_pushvalue(L, index_key);
      lua_rawget(L, index_meta);
      return 1;
   }
   int _dead_newindex(lua_State* L) {
      cobb::lua::error(L, "cannot set properties on a dead object (property name was %s)", lua_tostring(L, 2));
   }
   #pragma endregion

   void _create_zombie_sentinel_userdata(lua_State* L) {
      auto start = lua_gettop(L);
      lua_getfield(L, LUA_REGISTRYINDEX, zombie_sentinel_key);
      if (lua_isnoneornil(L, start + 1)) {
         lua_newuserdatauv(L, 1, 0);
         lua_setfield(L, LUA_REGISTRYINDEX, zombie_sentinel_key);
      }
      lua_settop(L, start);
   }

   int _to_zombie_class(lua_State* L) {
      /*
         local function _to_zombie_class(meta)
            local cci = meta.__custom_class_internals
            if cci then
               if cci.zombie_class then
                  return cci.zombie_class
               end
            else
               cci = {}
               meta.__custom_class_internals = cci
            end
            --
            local dead = {}
            dead.__index    = _dead_index
            dead.__newindex = _dead_newindex
            dead.__name     = "zombie<" .. meta.__name .. ">"
            dead.__iszombie = SENTINEL
            --
            local list = meta.__classlist
            if list then
               for i = #list, 1, -1 do
                  local cls = list[i]
                  local k, v = next(cls)
                  while k do
                     if type(v) == "function" and not _should_skip_name(k) then
                        dead[k] = _dead_method
                     end
                     k, v = next(cls)
                  end
               end
            end
            cci.zombie_class = dead
            return dead
         end
      */
      _create_zombie_sentinel_userdata(L);
      //
      constexpr int index_meta = 1;
      constexpr int index_cci  = 2;
      constexpr int index_dead = 3;
      constexpr int index_list = 4;
      constexpr int index_cls  = 5;
      //
      lua_settop(L, 1);
      cobb::lua::rawgetfield(L, 1, "__custom_class_internals");
      if (lua_istable(L, 2)) {
         if (LUA_TTABLE == cobb::lua::rawgetfield(L, 2, "zombie_class"))
            return 1;
         lua_pop(L, 1);
      } else {
         lua_pop(L, 1);
         lua_createtable(L, 0, 1);
         lua_pushvalue  (L, -1);
         cobb::lua::rawsetfield(L, index_meta, "__custom_class_internals");
      }
      //
      lua_createtable(L, 0, 3); // dead
      lua_pushcfunction(L, &_dead_index);
      cobb::lua::rawsetfield(L, index_dead, "__index");
      lua_pushcfunction(L, &_dead_newindex);
      cobb::lua::rawsetfield(L, index_dead, "__newindex");
      //
      lua_getfield(L, index_meta, "__name");
      lua_pushfstring(L, "zombie<%s>", lua_tostring(L, -1));
      cobb::lua::rawsetfield(L, index_dead, "__name");
      //
      lua_getfield(L, LUA_REGISTRYINDEX, zombie_sentinel_key);
      cobb::lua::rawsetfield(L, index_dead, "__iszombie");
      //
      if (LUA_TTABLE == cobb::lua::rawgetfield(L, index_meta, "__classlist")) {
         assert(lua_gettop(L) == index_list);
         auto len = lua_rawlen(L, index_list);
         for (decltype(len) i = 1; i <= len; ++i) {
            lua_rawgeti(L, index_list, i);
            lua_pushnil(L);
            while (lua_next(L, index_cls) != 0) {
               if (lua_type(L, -1) == LUA_TFUNCTION && !_should_skip_name(L, -2)) {
                  lua_pushvalue(L, -2);
                  lua_pushcfunction(L, &_dead_method);
                  lua_rawset(L, index_dead);
               }
               lua_pop(L, 1);
            }
         }
      }
      //
      lua_settop(L, index_dead);
      lua_pushvalue(L, -1);
      cobb::lua::rawsetfield(L, index_cci, "zombie_class");
      return 1;
   }
}

namespace dovahscript {
   int zombify_userdata(lua_State* L) { // call via lua_call, not directly
      /*
         function kill_class_of(ud)
            local meta = getmetatable(ud)
            if not meta then
               return
            end
            local dead = _to_zombie_class(meta)
            setmetatable(ud, dead)
         end
      */
      //
      auto index_ud    = 1;
      auto index_meta  = 2;
      auto index_dead  = 3;
      //
      lua_settop(L, 1);
      lua_getmetatable(L, index_ud); // 2
      if (lua_isnoneornil(L, index_meta))
         return 0;
      lua_pushcfunction(L, &_to_zombie_class);
      lua_pushvalue(L, index_meta);
      lua_call(L, 1, 1);
      assert(lua_gettop(L) == index_dead);
      assert(!lua_isnoneornil(L, index_dead));
      lua_setmetatable(L, index_ud);
      return 0;
   }

   extern bool userdata_is_zombie(lua_State* L, int stack_pos) {
      //
      // We mark zombie classes by setting an "__iszombie" field on them to point to a unique 
      // userdata that isn't accessed by anything else.  This means that even if a class or a 
      // userscript defines an "__iszombie" key on a metatable,  we can still tell that apart 
      // from a real zombie, because that key's value won't be our sentinel userdata.
      //
      if (lua_type(L, stack_pos) != LUA_TUSERDATA)
         return false;
      //
      auto start = lua_gettop(L);
      bool out   = false;
      //
      lua_getmetatable(L, stack_pos); // start + 1
      if (!lua_isnoneornil(L, start + 1)) {
         lua_getfield(L, start + 1, "__iszombie");                // start + 2
         lua_getfield(L, LUA_REGISTRYINDEX, zombie_sentinel_key); // start + 3 // NOTE: this will be nil if no zombies have been created yet!
         if (!lua_isnoneornil(L, start + 2) && !lua_isnoneornil(L, start + 3))
            out = lua_compare(L, start + 2, start + 3, LUA_OPEQ) == 1;
      }
      lua_settop(L, start);
      return out;
   }
}