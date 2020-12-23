#include "vector3.h"
#include "../util.h"
#include <cmath>

namespace {
   using namespace editor_script;

   classes::vector3& _get_this(lua_State* L) {
      auto* self = (classes::vector3*) editor_script::cast_to_class(L, 1, classes::vector3::metatable_key);
      if (self == nullptr) {
         luaL_error(L, "function called with bad self (expected %s)", classes::vector3::metatable_key);
      }
      __assume(self != nullptr);
      return *self;
   }
   classes::vector3& _get_vector_arg(lua_State* L, int i) {
      auto* arg = (classes::vector3*) editor_script::cast_to_class(L, i, classes::vector3::metatable_key);
      if (arg == nullptr) {
         luaL_error(L, "bad argument #%i (expected %s)", i, classes::vector3::metatable_key);
      }
      __assume(arg != nullptr);
      return *arg;
   }
   void _make_vector(lua_State* L, double x, double y, double z) { // creates a new Vector3 (as a normal table with "x", "y", and "z" members) on the top of the stack
      lua_createtable  (L, 0, 3);
      auto index = lua_gettop(L);
      luaL_getmetatable(L, classes::vector3::metatable_key);
      lua_setmetatable (L, index);
      lua_pushstring   (L, "x");
      lua_pushnumber   (L, x);
      lua_rawset       (L, index);
      lua_pushstring   (L, "y");
      lua_pushnumber   (L, y);
      lua_rawset       (L, index);
      lua_pushstring   (L, "z");
      lua_pushnumber   (L, z);
      lua_rawset       (L, index);
   }

   namespace _methods {
      luastackchange_t copy(lua_State* L) {
         auto& self = _get_this(L);
         lua_settop(L, 1);
         //
         double x, y, z;
         lua_pushstring(L, "x");
         lua_gettable  (L, 1);
         lua_pushstring(L, "y");
         lua_gettable  (L, 1);
         lua_pushstring(L, "z");
         lua_gettable  (L, 1);
         x = lua_tonumber(L, 3);
         y = lua_tonumber(L, 4);
         z = lua_tonumber(L, 5);
         //
         _make_vector(L, x, y, z);
         return 1;
      }
      luastackchange_t cross(lua_State* L) { // compute the cross product vector of two 3D vectors
         auto& self  = _get_this(L);
         auto& other = _get_vector_arg(L, 2);
         lua_settop(L, 2);
         //
         double ax, ay, az;
         double bx, by, bz;
         lua_pushstring(L, "x");
         lua_gettable  (L, 1);
         lua_pushstring(L, "y");
         lua_gettable  (L, 1);
         lua_pushstring(L, "z");
         lua_gettable  (L, 1);
         ax = lua_tonumber(L, 3);
         ay = lua_tonumber(L, 4);
         az = lua_tonumber(L, 5);
         lua_settop(L, 2);
         lua_pushstring(L, "x");
         lua_gettable  (L, 2);
         lua_pushstring(L, "y");
         lua_gettable  (L, 2);
         lua_pushstring(L, "z");
         lua_gettable  (L, 2);
         bx = lua_tonumber(L, 3);
         by = lua_tonumber(L, 4);
         bz = lua_tonumber(L, 5);
         lua_settop(L, 2);
         //
         double x = ay * bz - az * by;
         double y = az * bx - ax * bz;
         double z = ax * by - ay * bx;
         _make_vector(L, x, y, z);
         return 1;
      }
      luastackchange_t length(lua_State* L) {
         auto& self = _get_this(L);
         lua_settop(L, 1);
         //
         double x, y, z;
         lua_pushstring(L, "x");
         lua_gettable  (L, 1);
         lua_pushstring(L, "y");
         lua_gettable  (L, 1);
         lua_pushstring(L, "z");
         lua_gettable  (L, 1);
         x = lua_tonumber(L, 3);
         y = lua_tonumber(L, 4);
         z = lua_tonumber(L, 5);
         //
         double length = sqrt(x*x + y*y + z*z);
         lua_pushnumber(L, length);
         return 1;
      }
      luastackchange_t length_squared(lua_State* L) {
         auto& self = _get_this(L);
         lua_settop(L, 1);
         //
         double x, y, z;
         lua_pushstring(L, "x");
         lua_gettable  (L, 1);
         lua_pushstring(L, "y");
         lua_gettable  (L, 1);
         lua_pushstring(L, "z");
         lua_gettable  (L, 1);
         x = lua_tonumber(L, 3);
         y = lua_tonumber(L, 4);
         z = lua_tonumber(L, 5);
         //
         double length = x*x + y*y + z*z;
         lua_pushnumber(L, length);
         return 1;
      }
   }
}
namespace editor_script::classes {
   /*static*/ luaL_Reg vector3::metatable_methods[] = {
      //
      // TODO: Lua operator overload metamethods
      //  - __add
      //  - __div
      //  - __mul
      //  - __sub
      //
      { "copy",           &_methods::copy },
      { "cross",          &_methods::cross },
      { "length",         &_methods::length },
      { "length_squared", &_methods::length_squared },
      { nullptr, nullptr },
   };
}