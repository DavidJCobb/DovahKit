#include "classes.h"
#include <cassert>
#include <cstring>
#include "util.h"
#include "../../helpers/lua/metamethod_names.h"
#include "../../helpers/lua/setfuncs.h"

namespace editor_script {
   namespace __pairs_iterators { // code for __pairs iterators
      constexpr char* metatable_key = "-cobb-class-helpers:pairs-iterator";
      //
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
               else if (strcmp(nk, "__superclass") == 0)
                  return true;
               else if (strcmp(nk, "__name") == 0)
                  return true;
            }
            return false;
         }
      }
      static luastackchange_t __call(lua_State* L) {
         /*
         function(self, t, k)
            while true do
               local meta = self.meta
               if self.getters and meta then
                  meta = meta.__getters
               end
               --
               if meta then
                  local k, v = next(meta)
                  while _should_skip_name(k) do
                     k, v = next(meta)
                  end
                  if v ~= nil then
                     if self.getters then
                        v = (v)(t)
                     end
                     return k, v
                  end
               end
               --
               if not self.getters then
                  --
                  -- Move on to the getters.
                  --
                  self.getters = true
               else
                  --
                  -- Move on to the next superclass.
                  --
                  self.getters = false
                  meta = self.meta.__superclass
                  self.meta = meta
                  if not meta then
                     return
                  end
                  k = nil -- iterating new table, so must iterate from the start
               end
            end
         end
         */
         // STACK: - [ self, t, k ] +
         constexpr auto index_self = 1;
         constexpr auto index_tbl  = 2;
         constexpr auto index_key  = 3;
         constexpr auto index_meta = 4;
         constexpr auto index_nk   = 5;
         constexpr auto index_nv   = 6;
         do {
            assert(lua_gettop(L) == index_key);
            lua_getfield(L, index_self, "meta");
            lua_getfield(L, index_self, "getters");
            bool getters = lua_toboolean(L, -1);
            lua_settop(L, index_meta);
            //
            if (getters && !lua_isnoneornil(L, index_meta)) {
               lua_getfield(L, index_meta, "__getters");
               lua_replace (L, index_meta);
            }
            if (!lua_isnoneornil(L, index_meta)) {
               lua_pushvalue(L, index_key); // STACK: - [ self, t, k, meta, nk ] +
               while (lua_next(L, index_meta) != 0) {
                  if (!_should_skip_name(L, index_nk)) {
                     if (getters) {
                        //
                        // Execute the getter.
                        //
                        lua_pushvalue(L, index_tbl);
                        lua_call(L, 1, 1);
                     }
                     return 2;
                  }
                  lua_settop(L, index_nk);
               }
            }
            //
            if (!getters) {
               //
               // Move on to the getters.
               //
               lua_pushboolean(L, true);
               lua_setfield(L, index_self, "getters");
            } else {
               //
               // Move on to the next superclass.
               //
               lua_pushboolean(L, false);
               lua_setfield(L, index_self, "getters");
               //
               lua_settop(L, index_key); // get the (meta) value again, since we may have overwritten the stack position with (meta.__getters)
               lua_getfield(L, index_self, "meta");
               lua_getfield(L, index_meta, "__superclass");
               if (lua_isnoneornil(L, -1))
                  return 0;
               lua_setfield(L, index_self, "meta");
            }
            lua_settop(L, index_key - 1);
            lua_pushnil(L); // set the key to nil since we're now iterating a new table
         } while (true);
      }
      //
      void _define_metatable(lua_State* L) {
         auto start   = lua_gettop(L);
         bool defined = luaL_getmetatable(L, metatable_key) == LUA_TTABLE;
         lua_settop(L, start);
         if (defined)
            return;
         luaL_newmetatable(L, metatable_key);
         auto index_mt = start + 1;
         //
         lua_pushcfunction(L, &__call);
         lua_setfield(L, index_mt, "__call");
         //
         lua_settop(L, start);
      }
   }

   namespace { // member functions for the class metatables
      static luastackchange_t __index(lua_State* luaVM) {
         //
         // LUA:
         //    function __index(t, k)
         //       local meta = getmetatable(t)
         //       if not meta then
         //          return nil
         //       end
         //       while true do
         //          local a = rawget(meta, k)
         //          if a then
         //             return a
         //          end
         //          --
         //          -- Check for getters:
         //          --
         //          a = rawget(meta, "__getters")
         //          if a then
         //             a = rawget(a, k)
         //             if a then
         //                return a(t) -- remember: selfcall is just self as arg 1
         //             end
         //          end
         //          --
         //          -- Traverse up to the next superclass:
         //          --
         //          meta = meta.__superclass
         //          if not meta then
         //             return
         //          end
         //       end
         //    end
         //
         auto index_table = 1;
         auto index_key   = 2;
         if (!lua_getmetatable(luaVM, index_table)) // STACK: - [ t, k, meta ] +
            return 0;
         auto index_meta  = 3;
         #if _DEBUG
            const char* __key = lua_tostring(luaVM, index_key);
         #endif
         //
         do {
            //
            // Check for a member:
            //
            lua_pushvalue(luaVM, index_key);  // STACK: - [ t, k, meta, k       ] +
            lua_rawget   (luaVM, index_meta); // STACK: - [ t, k, meta, meta[k] ] +
            if (!lua_isnoneornil(luaVM, -1))
               return 1;
            lua_settop(luaVM, index_meta); // STACK: - [ t, k, meta ] +
            //
            // Check for a getter:
            //
            lua_pushstring(luaVM, "__getters"); // STACK: - [ t, k, meta, "__getters" ] +
            if (lua_rawget(luaVM, index_meta) == LUA_TTABLE) { // STACK: - [ t, k, meta, meta.__getters ] +
               lua_pushvalue(luaVM, index_key); // STACK: - [ t, k, meta, meta.__getters, k ] +
               lua_rawget   (luaVM, -2);        // STACK: - [ t, k, meta, meta.__getters, meta.__getters[k] ] +
               if (!lua_isnoneornil(luaVM, -1)) {
                  lua_copy  (luaVM, -1, 2); // STACK: - [ t, meta.__getters[k], meta, meta.__getters, meta.__getters[k] ] +
                  lua_settop(luaVM, 2);     // STACK: - [ t, meta.__getters[k] ] +
                  lua_rotate(luaVM, 1, 1);  // STACK: - [ meta.__getters[k], t ] +
                  lua_call  (luaVM, 1, 1);  // STACK: - [ meta.__getters[k](t) ] +
                  //
                  // Quick explanation for my own reference, since basically only one page on the entire Internet 
                  // has documented this and it's not the Lua manual: given the stack
                  //
                  //     A B C D E
                  //     1 2 3 4 5
                  //
                  // A call to lua_rotate(L, 3, 1) will produce:
                  //
                  //     A B | E C D
                  //     1 2 | 3 4 5
                  //
                  // Positive offsets rotate right; negative, left.
                  //
                  return 1;
               }
            }
            lua_settop(luaVM, index_meta); // STACK: - [ t, k, meta ] +
            //
            // Traverse up to the next superclass:
            //
            lua_pushstring(luaVM, "__superclass");
            if (lua_rawget(luaVM, index_meta) == LUA_TTABLE) { // STACK: - [ t, k, v, meta, meta.__superclass ] +
               lua_remove(luaVM, index_meta); // STACK: - [ t, k, v, meta.__superclass ] + // meta = meta.__superclass;
            } else {
               break;
            }
         } while (true);
         return 0;
      }
      static luastackchange_t __newindex(lua_State* luaVM) {
         //
         // Code to power setters for userdata.
         //
         // LUA:
         //    function __newindex(t, k, v)
         //       local meta = getmetatable(t)
         //       if not meta then
         //          return nil
         //       end
         //       local has_getter = false
         //       while true do
         //          local a = meta.__setters
         //          if a then
         //             a = a[k]
         //             if a then
         //                a(t, v)
         //                return
         //             end
         //          end
         //          if not has_getter then
         //             a = meta.__getters
         //             if a and a[k] then
         //                has_getter = true
         //             end
         //          end
         //          --
         //          -- Traverse up to the next superclass:
         //          --
         //          meta = meta.__superclass
         //          if not meta then
         //             break
         //          end
         //       end
         //       if has_getter then
         //          error(string.format("DovahKit does not allow you to assign to property '%s' on this class", k))
         //       end
         //       error(string.format("class %s does not offer a property named '%s'", getmetatable(t).__name, k))
         //    end
         //
         auto index_table = 1;
         auto index_key   = 2;
         auto index_value = 3;
         if (!lua_getmetatable(luaVM, index_table)) // STACK: - [ t, k, v, meta ] +
            return 0;
         auto index_meta  = 4;
         #if _DEBUG
            const char* __key = lua_tostring(luaVM, index_key);
         #endif
         //
         bool has_getter = false;
         //
         do {
            lua_pushstring(luaVM, "__setters"); // STACK: - [ t, k, v, meta, "__setters" ] +
            if (lua_rawget(luaVM, index_meta) == LUA_TTABLE) { // STACK: - [ t, k, v, meta, meta.__setters ] +
               lua_pushvalue(luaVM, index_key); // STACK: - [ t, k, v, meta, meta.__setters, k ] +
               lua_rawget   (luaVM, -2);        // STACK: - [ t, k, v, meta, meta.__setters, meta.__setters[k] ] +
               if (!lua_isnoneornil(luaVM, -1)) {
                  lua_copy  (luaVM,  1, 2); // STACK: - [ t, t, v, meta, meta.__setters, meta.__setters[k] ] +
                  lua_copy  (luaVM, -1, 1); // STACK: - [ meta.__setters[k], t, v, meta, meta.__setters, meta.__setters[k] ] +
                  lua_settop(luaVM,  3);    // STACK: - [ meta.__setters[k], t, v ] +
                  lua_call  (luaVM, 2, 0);
                  return 0;
               }
            }
            lua_settop(luaVM, index_meta); // STACK: - [ t, k, v, meta ] +
            if (!has_getter) {
               lua_pushstring(luaVM, "__getters");
               if (lua_rawget(luaVM, index_meta) == LUA_TTABLE) { // STACK: - [ t, k, v, meta, meta.__getters ] +
                  lua_pushvalue(luaVM, index_key); // STACK: - [ t, k, v, meta, meta.__getters, k ] +
                  lua_rawget   (luaVM, -2);        // STACK: - [ t, k, v, meta, meta.__getters, meta.__getters[k] ] +
                  if (!lua_isnoneornil(luaVM, -1)) {
                     has_getter = true;
                  }
               }
               lua_settop(luaVM, index_meta); // STACK: - [ t, k, v, meta ] +
            }
            //
            // Traverse up to the next superclass:
            //
            lua_pushstring(luaVM, "__superclass");
            if (lua_rawget(luaVM, index_meta) == LUA_TTABLE) { // STACK: - [ t, k, v, meta, meta.__superclass ] +
               lua_remove(luaVM, index_meta); // STACK: - [ t, k, v, meta.__superclass ] + // meta = meta.__superclass;
            } else {
               break;
            }
         } while (true);
         //
         const char* key       = lua_tostring(luaVM, index_key);
         const char* classname = "?";
         lua_settop      (luaVM, index_value);
         lua_getmetatable(luaVM, index_table); // STACK: - [ t, k, v, meta ] +
         lua_pushstring  (luaVM, "__name");
         if (lua_rawget(luaVM, index_meta) == LUA_TSTRING) {
            classname = lua_tostring(luaVM, -1);
         }
         if (has_getter) {
            luaL_error(luaVM, "DovahKit does not allow you to assign to property '%s' on class %s", key, classname);
            __assume(0); // unreachable
         }
         luaL_error(luaVM, "class %s does not offer a property named '%s'", classname, key);
         return 0;
      }
      static luastackchange_t __pairs(lua_State* L) {
         lua_settop(L, 1);
         //
         lua_createtable(L, 0, 2); // index 2 (iterator)
         lua_getmetatable(L, 1);
         lua_setfield(L, 2, "meta");
         lua_pushboolean(L, false);
         lua_setfield(L, 2, "getters");
         luaL_getmetatable(L, __pairs_iterators::metatable_key);
         lua_setmetatable(L, 2);
         //
         lua_pushvalue(L, 1);
         lua_pushnil(L);
         return 3;
      }
   }
   namespace { // helper functions
      //
      // Lua only applies "operator" metamethods, like __tostring, using rawget, so 
      // they aren't automatically inherited by subclasses. This helper function will 
      // copy a desired metamethod (or any member, in fact) from a superclass to a 
      // subclass.
      //
      void _forward_metamethod_to_subclass(lua_State* luaVM, const char* metamethod, int superPos, int subPos) {
         superPos = lua_absindex(luaVM, superPos);
         subPos   = lua_absindex(luaVM, subPos);
         //
         // Don't overwrite the metamethod if it already exists on the subclass metatable:
         //
         lua_pushstring(luaVM, metamethod);
         if (lua_rawget(luaVM, subPos) != LUA_TNIL) {
            lua_pop(luaVM, 1);
            return;
         }
         lua_pop(luaVM, 1);
         //
         // Copy the metamethod from the superclass table to the subclass table, if it 
         // exists on the former:
         //
         lua_pushstring(luaVM, metamethod); // STACK: [metamethod, ...]
         lua_pushstring(luaVM, metamethod); // STACK: [metamethod, metamethod, ...]
         lua_rawget    (luaVM, superPos);   // STACK: [super[metamethod], metamethod, ...]
         if (!lua_isnil(luaVM, -1))
            lua_rawset(luaVM, subPos); // STACK: [...]
         else
            lua_pop(luaVM, 2);
      }
   }

   extern void* cast_to_class(lua_State* luaVM, int stackPos, const char* classKey) {
      //
      // LUA:
      //    if not userdata then
      //       return nil
      //    end
      //    local instanceMeta = getmetatable(userdata)
      //    local classMeta    = getmetatable(classKey)
      //    while classMeta ~= instanceMeta do
      //       local a = instanceMeta.__superclass
      //       if type(a) ~= "table" then
      //          return nil
      //       end
      //       instanceMeta = a
      //    end
      //    return userdata
      //
      lua_checkstack(luaVM, 3);
      auto top = lua_gettop(luaVM);
      //
      void* userdata = lua_touserdata(luaVM, stackPos);
      if (!userdata)
         //
         // TODO: Every case in which we return nullptr should also log an error.
         //
         return nullptr;
      if (!lua_getmetatable(luaVM, stackPos))
         return nullptr;
      luaL_getmetatable(luaVM, classKey);
      // STACK: [classMeta, instanceMeta, ...]
      while (!lua_rawequal(luaVM, -1, -2)) {
         lua_pushstring(luaVM, "__superclass"); // STACK: ["__superclass", classMeta, instanceMeta, ...]
         auto type = lua_rawget(luaVM, -3); // STACK: [classMeta["__superclass"], classMeta, instanceMeta, ...]
         if (type != LUA_TTABLE) {
            lua_settop(luaVM, top);
            return nullptr;
         }
         lua_replace(luaVM, -3); // STACK: [classMeta, classMeta["__superclass"], ...]
      }
      lua_settop(luaVM, top);
      return userdata;
   }
   
   extern void* cast_to_exact_class(lua_State* luaVM, int stackPos, const char* classKey) {
      //
      // LUA:
      //    if not userdata then
      //       return nil
      //    end
      //    local instanceMeta = getmetatable(userdata)
      //    local classMeta    = getmetatable(classKey)
      //    if classMeta ~= instanceMeta do
      //       return nil
      //    end
      //    return userdata
      //
      lua_checkstack(luaVM, 2);
      auto  top = lua_gettop(luaVM);
      void* ud  = lua_touserdata(luaVM, stackPos);
      if (!ud)
         return nullptr;
      if (!lua_getmetatable(luaVM, stackPos))
         return nullptr;
      luaL_getmetatable(luaVM, classKey);
      // STACK: [classMeta, instanceMeta, ...]
      if (!lua_rawequal(luaVM, -1, -2)) {
         lua_settop(luaVM, top);
         return nullptr;
      }
      lua_settop(luaVM, top);
      return ud;
   }

   extern bool check_for_class(lua_State* luaVM, int stack_pos, const char* class_internal_name) {
      //
      // LUA:
      //    local meta = getmetatable(t)
      //    if not meta then
      //       return false
      //    end
      //    local cls  = getmetatable(classKey)
      //    while cls ~= meta do
      //       local a = meta.__superclass
      //       if type(a) ~= "table" then
      //          return false
      //       end
      //       meta = a
      //    end
      //    return true
      //
      lua_checkstack(luaVM, 3);
      auto top = lua_gettop(luaVM);
      //
      if (!lua_getmetatable(luaVM, stack_pos))
         return false;
      luaL_getmetatable(luaVM, class_internal_name); // STACK: - [ ..., meta, cls ] +
      while (!lua_rawequal(luaVM, -1, -2)) {
         if (lua_getfield(luaVM, -2, "__superclass") != LUA_TTABLE) { // STACK: - [ ..., meta, cls, meta.__superclass ] +
            lua_settop(luaVM, top);
            return false;
         }
         lua_replace(luaVM, -3);
      }
      lua_settop(luaVM, top);
      return true;
   }

   extern void define_class(lua_State* luaVM, const char* className, const char* superclassName, const std::initializer_list<luaL_Reg>& methods, const std::initializer_list<luaL_Reg>& getters, const std::initializer_list<luaL_Reg>& setters) {
      //
      // LUA:
      //    local meta = {}
      //    registry[className] = meta
      //    --
      //    meta.__index = __index -- CFunction
      //    --
      //    if superclassName then
      //       meta.__superclass = registry[superclassName]
      //       --
      //       -- "Operator" metamethods need to be inherited manually:
      //       --
      //       meta.__tostring = meta.__superclass.__tostring
      //    end
      //    --
      //    if methods then
      //       for k, v in pairs(methods) do
      //          meta[k] = v
      //       end
      //    end
      //    if getters then
      //       meta.__getters = {}
      //       for k, v in pairs(methods) do
      //          meta.__getters[k] = v
      //       end
      //    end
      //    if setters then
      //       meta.__setters = {}
      //       for k, v in pairs(methods) do
      //          meta.__setters[k] = v
      //       end
      //    end
      //
      //
      __pairs_iterators::_define_metatable(luaVM); // needed for __pairs
      if (superclassName)
         assert(strcmp(className, superclassName) != 0 && "The superclass and subclass can't use the same registry key name."); // (assert) should be no-op in Release, so this is fine
      lua_checkstack(luaVM, 3);
      //
      luaL_newmetatable(luaVM, className); // STACK: [newmeta]
      auto index_mt = lua_gettop(luaVM);
      //
      lua_pushstring   (luaVM, "__index"); // STACK: ["__index", newmeta]
      lua_pushcfunction(luaVM, &__index);  // STACK: [CFunction:__index, "__index", newmeta]
      lua_settable     (luaVM, index_mt);  // STACK: [newmeta]
      lua_pushstring   (luaVM, "__pairs"); // STACK: ["__index", newmeta]
      lua_pushcfunction(luaVM, &__pairs);  // STACK: [CFunction:__index, "__index", newmeta]
      lua_settable     (luaVM, index_mt);  // STACK: [newmeta]
      //
      if (superclassName) {
         luaL_getmetatable(luaVM, superclassName); // STACK: [supermeta, newmeta]
         assert(!lua_isnil(luaVM, -1) && "The desired superclass doesn't yet have a metatable set up. Are you setting up your classes in the wrong order?");
         lua_pushstring(luaVM, "__superclass"); // STACK: ["__superclass", supermeta, newmeta]
         lua_pushvalue (luaVM, -2); // STACK: [supermeta, "__superclass", supermeta, newmeta]
         lua_settable  (luaVM, index_mt); // STACK: [supermeta, newmeta]
         //
         // Lua only applies "operator" metamethods using rawget, so we can't rely 
         // on classes to inherit them automatically. We need to copy  them from 
         // the superclass to the subclass by hand. We have a helper function for 
         // this.
         //
         // The __gc metamethod also needs to be forwarded for userdata.
         //
         _forward_metamethod_to_subclass(luaVM, "__tostring", -1, -2);
         _forward_metamethod_to_subclass(luaVM, "__gc", -1, -2);
         //
         lua_pop(luaVM, 1); // STACK: [newmeta]
      }
      //
      if (methods.size()) {
         cobb::lua::setfuncs(luaVM, methods); // import functions into the metatable
      }
      if (getters.size()) {
         lua_pushstring (luaVM, "__getters"); // push 1
         lua_createtable(luaVM, 0, 0);        // push 1
         cobb::lua::setfuncs(luaVM, getters); // push 0
         lua_settable   (luaVM, index_mt);    // pop  2
      }
      if (setters.size()) {
         lua_pushstring   (luaVM, "__newindex"); // push 1
         lua_pushcfunction(luaVM, &__newindex);  // push 1
         lua_settable     (luaVM, -3);           // pop  2
         //
         lua_pushstring (luaVM, "__setters"); // push 1
         lua_createtable(luaVM, 0, 0);        // push 1
         cobb::lua::setfuncs(luaVM, setters); // push 0
         lua_settable   (luaVM, index_mt);    // pop  2
      }
      lua_pop(luaVM, 1); // pop metatable from the stack
   }

   extern bool is_class_defined(lua_State* luaVM, const char* className) {
      bool result = luaL_getmetatable(luaVM, className) == LUA_TTABLE;
      lua_pop(luaVM, 1);
      return result;
   }
}