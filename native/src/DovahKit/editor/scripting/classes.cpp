#include "classes.h"
#include <cassert>
#include "util.h"

namespace editor_script {
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
   extern void define_class(lua_State* luaVM, const char* className, const char* superclassName, const luaL_Reg* methods) {
      //
      // LUA:
      //    local meta = {}
      //    registry[className] = meta
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
      //
      //
      lua_checkstack(luaVM, 3);
      //
      luaL_newmetatable(luaVM, className); // STACK: [newmeta]
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
         _forward_metamethod_to_subclass(luaVM, "__tostring", -1, -2);
         //
         lua_pop(luaVM, 1); // STACK: [newmeta]
      }
      //
      if (methods)
         luaL_setfuncs(luaVM, methods, 0); // import functions into the metatable
      lua_pop(luaVM, 1); // pop metatable from the stack
   }
   extern bool is_class_defined(lua_State* luaVM, const char* className) {
      bool result = luaL_getmetatable(luaVM, className) == LUA_TTABLE;
      lua_pop(luaVM, 1);
      return result;
   }
}