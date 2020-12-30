#include "euler.h"
#include "../util.h"
#include <cmath>

#include "../../../helpers/rotation.h"
#include "matrix3x3.h"

namespace {
   using namespace editor_script;
   using cls = classes::euler;

   namespace _methods {
      luastackchange_t __tostring(lua_State* L) { // creates and returns new vector
         lua_settop(L, 1);
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         double x = cobb::radians_to_degrees(lua_tonumber(L, 2));
         double y = cobb::radians_to_degrees(lua_tonumber(L, 3));
         double z = cobb::radians_to_degrees(lua_tonumber(L, 4));
         lua_pushfstring(L, "(%fdeg, %fdeg, %fdeg)", x, y, z);
         return 1;
      }
      luastackchange_t copy(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         lua_Number x, y, z;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         x = lua_tonumber(L, 3);
         y = lua_tonumber(L, 4);
         z = lua_tonumber(L, 5);
         //
         cls::push_new_instance(L, { x, y, z });
         return 1;
      }
      luastackchange_t to_matrix(lua_State* L) { // compute the cross product vector of two 3D vectors
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         auto converted = (cobb::rotation_matrix) cobb::euler{ lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) };
         lua_settop(L, 1);
         //
         classes::matrix3x3::push_new_instance(L, converted);
         return 1;
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_obj(lua_State* L) {
         //
         // function euler:new(a, b, c)
         //    if tonumber(a) then
         //       return _make_instance(a, b, c)
         //    end
         //    if type(a) == "table" or type(a) == "userdata" then
         //       return _make_instance(a.x or 0, a.y or 0, a.z or 0)
         //    end
         //    return _make_instance(0, 0, 0)
         // end
         //
         if (lua_isnumber(L, 1)) {
            cls::push_new_instance(L, { lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3) });
            return 1;
         }
         auto rawtype = lua_type(L, 1);
         if (rawtype == LUA_TTABLE || rawtype == LUA_TUSERDATA) {
            lua_settop(L, 1);
            lua_getfield(L, 1, "x");
            lua_getfield(L, 1, "y");
            lua_getfield(L, 1, "z");
            cls::push_new_instance(L, { lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) });
            return 1;
         }
         cls::push_new_instance(L, {});
         return 1;
      }
      luastackchange_t is(lua_State* L) {
         if (cls::check_arg_type(L, 1)) {
            lua_pushboolean(L, 1);
            return 1;
         }
         lua_pushboolean(L, 0);
         return 1;
      }
   }
}
namespace editor_script::classes {
   /*static*/ std::initializer_list<luaL_Reg> euler::metatable_methods = {
      { "__tostring", &_methods::__tostring },
      { "copy",       &_methods::copy },
      { "to_matrix",  &_methods::to_matrix },
   };

   /*static*/ void euler::push_new_instance(lua_State* L, const cobb::euler& raw) {
      lua_createtable  (L, 0, 3);
      auto index = lua_gettop(L);
      luaL_getmetatable(L, cls::metatable_key);
      lua_setmetatable (L, index);
      lua_pushstring   (L, "x");
      lua_pushnumber   (L, raw.x);
      lua_rawset       (L, index);
      lua_pushstring   (L, "y");
      lua_pushnumber   (L, raw.y);
      lua_rawset       (L, index);
      lua_pushstring   (L, "z");
      lua_pushnumber   (L, raw.z);
      lua_rawset       (L, index);
   }

   /*static*/ void euler::setup(lua_State* L) {
      editor_script::define_class(L, metatable_key, nullptr, metatable_methods);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_obj);
      lua_setfield(L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield(L, -2, "is");
      lua_setglobal(L, cls::global_name);
   }
}