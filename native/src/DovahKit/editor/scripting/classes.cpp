#include "classes.h"
#include <cassert>
#include <cstring>
#include "util.h"

namespace editor_script {
   namespace { // member functions for the class metatables
      static luastackchange_t __index(lua_State* luaVM) {
         //
         // We need to explicitly reproduce normal table behavior for userdata; 
         // it's not built-in.
         //
         // LUA:
         //    function __index(t, k)
         //       local meta = getmetatable(t)
         //       if not meta then
         //          return nil
         //       end
         //       local a = rawget(meta, k)
         //       if a then
         //          return a
         //       end
         //       --
         //       -- Check for any getters:
         //       --
         //       a = rawget(meta, "__getters")
         //       if a then
         //          a = rawget(a, k)
         //          if a then
         //             return a(t) -- remember: selfcall is just self as arg 1
         //          end
         //       end
         //       --
         //       -- Traverse up the superclass chain:
         //       --
         //       while true do
         //          local b = rawget(meta, "__superclass")
         //          if type(b) ~= "table" then
         //             return nil
         //          end
         //          meta = b
         //          local c = rawget(meta, k)
         //          if c then
         //             return c
         //          end
         //          c = rawget(meta, "__getters")
         //          if c then
         //             c = rawget(c, k)
         //             if c then
         //                return c(t) -- remember: selfcall is just self as arg 1
         //             end
         //          end
         //       end
         //    end
         //
         auto index_table = 1;
         auto index_key   = 2;
         if (!lua_getmetatable(luaVM, index_table)) // STACK: - [ t, k, meta ] +
            return 0;
         auto index_meta  = 3;
         lua_pushvalue(luaVM, index_key);  // STACK: - [ t, k, meta, k       ] +
         lua_rawget   (luaVM, index_meta); // STACK: - [ t, k, meta, meta[k] ] +
         if (!lua_isnil(luaVM, -1))
            return 1;
         lua_settop(luaVM, index_meta); // STACK: - [ t, k, meta ] +
         //
         // Check for any getters:
         //
         lua_pushstring(luaVM, "__getters"); // STACK: - [ t, k, meta, "__getters" ] +
         if (lua_rawget(luaVM, index_meta) == LUA_TTABLE) { // STACK: - [ t, k, meta, meta.__getters ] +
            lua_pushvalue(luaVM, index_key); // STACK: - [ t, k, meta, meta.__getters, k ] +
            lua_rawget   (luaVM, -2);        // STACK: - [ t, k, meta, meta.__getters, meta.__getters[k] ] +
            if (!lua_isnil(luaVM, -1)) {
               lua_copy  (luaVM, -1, 2); // STACK: - [ t, meta.__getters[k], meta, meta.__getters, meta.__getters[k] ] +
               lua_settop(luaVM, 2);     // STACK: - [ t, meta.__getters[k] ] +
               lua_rotate(luaVM, 2, 1);  // STACK: - [ meta.__getters[k], t ] +
               lua_call  (luaVM, 1, 1);  // STACK: - [ meta.__getters[k](t) ] +
               return 1;
            }
         }
         lua_settop(luaVM, index_meta); // STACK: - [ t, k, meta ] +
         //
         // Traverse up the superclass chain:
         //
         while (true) {
            lua_pushstring(luaVM, "__superclass");           // STACK: - [ t, k, meta, "__superclass"    ] +
            if (lua_rawget(luaVM, index_meta) != LUA_TTABLE) // STACK: - [ t, k, meta, meta.__superclass ] +
               return 0;
            lua_remove(luaVM, index_meta); // STACK: - [ t, k, meta.__superclass ] + // meta = meta.__superclass;
            //
            lua_pushvalue(luaVM, index_key);  // STACK: - [ t, k, meta, k       ] +
            lua_rawget   (luaVM, index_meta); // STACK: - [ t, k, meta, meta[k] ] +
            if (!lua_isnil(luaVM, -1))
               return 1;
            lua_settop    (luaVM, index_meta);  // STACK: - [ t, k, meta              ] +
            lua_pushstring(luaVM, "__getters"); // STACK: - [ t, k, meta, "__getters" ] +
            if (lua_rawget(luaVM, index_meta) == LUA_TTABLE) { // STACK: - [ t, k, meta, meta.__getters ] +
               lua_pushvalue(luaVM, index_key); // STACK: - [ t, k, meta, meta.__getters, k ] +
               lua_rawget   (luaVM, -2);        // STACK: - [ t, k, meta, meta.__getters, meta.__getters[k] ] +
               if (!lua_isnil(luaVM, -1)) {
                  lua_copy  (luaVM, -1, 2); // STACK: - [ t, meta.__getters[k], meta, meta.__getters, meta.__getters[k] ] +
                  lua_settop(luaVM, 2);     // STACK: - [ t, meta.__getters[k] ] +
                  lua_rotate(luaVM, 2, 1);  // STACK: - [ meta.__getters[k], t ] +
                  lua_call  (luaVM, 1, 1);  // STACK: - [ meta.__getters[k](t) ] +
                  return 1;
               }
            }
            lua_settop(luaVM, index_meta); // STACK: - [ t, k, meta ] +
         }
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
         //       local a = meta.__setters
         //       if a then
         //          a = a[k]
         //          if a then
         //             a(t, v)
         //          end
         //       end
         //    end
         //
         auto index_table = 1;
         auto index_key   = 2;
         auto index_value = 3;
         if (!lua_getmetatable(luaVM, index_table)) // STACK: - [ t, k, v, meta ] +
            return 0;
         auto index_meta  = 4;
         lua_pushstring(luaVM, "__setters"); // STACK: - [ t, k, v, meta, "__setters" ] +
         if (lua_rawget(luaVM, index_meta) == LUA_TTABLE) { // STACK: - [ t, k, v, meta, meta.__setters ] +
            lua_pushvalue(luaVM, index_key); // STACK: - [ t, k, v, meta, meta.__setters, k ] +
            lua_rawget   (luaVM, -2);        // STACK: - [ t, k, v, meta, meta.__setters, meta.__setters[k] ] +
            if (!lua_isnil(luaVM, -1)) {
               lua_copy  (luaVM,  1, 2); // STACK: - [ t, t, v, meta, meta.__setters, meta.__setters[k] ] +
               lua_copy  (luaVM, -1, 1); // STACK: - [ meta.__setters[k], t, v, meta, meta.__setters, meta.__setters[k] ] +
               lua_settop(luaVM,  3);    // STACK: - [ meta.__setters[k], t, v ] +
               lua_call  (luaVM, 2, 0);
               return 0;
            }
         }
         return 0;
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
   extern void define_class(lua_State* luaVM, const char* className, const char* superclassName, const luaL_Reg* methods, const luaL_Reg* getters, const luaL_Reg* setters) {
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
      if (superclassName)
         assert(strcmp(className, superclassName) != 0 && "The superclass and subclass can't use the same registry key name."); // (assert) should be no-op in Release, so this is fine
      lua_checkstack(luaVM, 3);
      //
      luaL_newmetatable(luaVM, className); // STACK: [newmeta]
      //
      lua_pushstring   (luaVM, "__index"); // STACK: ["__index", newmeta]
      lua_pushcfunction(luaVM, &__index);  // STACK: [CFunction:__index, "__index", newmeta]
      lua_settable     (luaVM, -3);        // STACK: [newmeta]
      //
      if (superclassName) {
         luaL_getmetatable(luaVM, superclassName); // STACK: [supermeta, newmeta]
         assert(!lua_isnil(luaVM, -1) && "The desired superclass doesn't yet have a metatable set up. Are you setting up your classes in the wrong order?");
         lua_pushstring(luaVM, "__superclass"); // STACK: ["__superclass", supermeta, newmeta]
         lua_pushvalue (luaVM, -2); // STACK: [supermeta, "__superclass", supermeta, newmeta]
         lua_settable  (luaVM, -4); // STACK: [supermeta, newmeta]
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
      if (methods) {
         luaL_setfuncs(luaVM, methods, 0); // import functions into the metatable
      }
      if (getters) {
         lua_pushstring (luaVM, "__getters"); // push 1
         lua_createtable(luaVM, 0, 0);        // push 1
         luaL_setfuncs  (luaVM, getters, 0);  // push 0
         lua_settable   (luaVM, -3);          // pop  2
      }
      if (setters) {
         lua_pushstring   (luaVM, "__newindex"); // push 1
         lua_pushcfunction(luaVM, &__newindex);  // push 1
         lua_settable     (luaVM, -3);           // pop  2
         //
         lua_pushstring (luaVM, "__setters"); // push 1
         lua_createtable(luaVM, 0, 0);        // push 1
         luaL_setfuncs  (luaVM, setters, 0);  // push 0
         lua_settable   (luaVM, -3);          // pop  2
      }
      lua_pop(luaVM, 1); // pop metatable from the stack
   }

   extern bool is_class_defined(lua_State* luaVM, const char* className) {
      bool result = luaL_getmetatable(luaVM, className) == LUA_TTABLE;
      lua_pop(luaVM, 1);
      return result;
   }
}