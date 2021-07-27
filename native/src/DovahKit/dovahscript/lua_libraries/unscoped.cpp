#include "unscoped.h"
#include "../core/classes.h"
#include "../core/class_zombification.h"
#include "../wrappers/form/form.h"

namespace {
   namespace _functions {
      int object_is_form(lua_State* L) {
         lua_settop(L, 1);
         bool value = false;
         if (lua_type(L, 1) == LUA_TUSERDATA) {
            value = dovahscript::classes::cast_to_class(L, 1, dovahscript::wrappers::form::metatable_key) != nullptr;
         }
         lua_pushboolean(L, value);
         return 1;
      }
      int object_is_zombie(lua_State* L) {
         lua_settop(L, 1);
         lua_pushboolean(L, dovahscript::userdata_is_zombie(L, 1));
         return 1;
      }
   }
}

namespace dovahscript::lua_libraries {
   namespace unscoped {
      extern void import(lua_State* L) {
         {  // object_is_form
            auto ti = lua_gettop(L);
            lua_pushstring(L, "object_is_form");
            lua_pushcfunction(L, &_functions::object_is_form);
            lua_rawset(L, ti);
         }
         {  // object_is_zombie
            auto ti = lua_gettop(L);
            lua_pushstring(L, "object_is_zombie");
            lua_pushcfunction(L, &_functions::object_is_zombie);
            lua_rawset(L, ti);
         }
      }
   }
}