#include "vector3.h"
#include "../util.h"
#include <cmath>

namespace {
   using namespace editor_script;
   using cls = classes::vector3;

   int _simple_vector_operator_overload(lua_State* L, int op) { // returns new vector
      cls::require_self_type(L);
      auto rawtype = lua_type(L, 2);
      if (rawtype != LUA_TTABLE && rawtype != LUA_TUSERDATA) {
         luaL_error(L, "bad argument #%i (expected table or userdata)", 2);
         __assume(0);
      }
      //
      lua_getfield(L, 1, "x");
      lua_getfield(L, 2, "x");
      lua_arith(L, op);
      //
      lua_getfield(L, 1, "y");
      lua_getfield(L, 2, "y");
      lua_arith(L, op);
      //
      lua_getfield(L, 1, "z");
      lua_getfield(L, 2, "z");
      lua_arith(L, op);
      //
      cls::push_new_instance(L, lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
      return 1;
   }
   int _simple_vector_assign_operator_overload(lua_State* L, int op) { // modifies and returns self
      cls::require_self_type(L);
      auto rawtype = lua_type(L, 2);
      if (rawtype != LUA_TTABLE && rawtype != LUA_TUSERDATA) {
         luaL_error(L, "bad argument #%i (expected table or userdata)", 2);
         __assume(0);
      }
      //
      lua_getfield(L, 1, "x");
      lua_getfield(L, 2, "x");
      lua_arith(L, op);
      lua_setfield(L, 1, "x");
      //
      lua_getfield(L, 1, "y");
      lua_getfield(L, 2, "y");
      lua_arith(L, op);
      lua_setfield(L, 1, "y");
      //
      lua_getfield(L, 1, "z");
      lua_getfield(L, 2, "z");
      lua_arith(L, op);
      lua_setfield(L, 1, "z");
      //
      lua_settop(L, 1); // return (self) to allow chaining
      return 1;
   }

   namespace _methods {
      luastackchange_t __add(lua_State* L) { // creates and returns new vector
         return _simple_vector_operator_overload(L, LUA_OPADD);
      }
      luastackchange_t __div(lua_State* L) { // creates and returns new vector
         cls::require_self_type(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected non-zero number");
         auto scalar = lua_tonumber(L, 2);
         if (scalar == 0.0)
            luaL_error(L, "bad argument #%i (expected non-zero number; got zero)", 2);
         //
         lua_getfield  (L, 1, "x");
         lua_pushnumber(L, scalar);
         lua_arith     (L, LUA_OPDIV); // 3
         lua_getfield  (L, 1, "y");
         lua_pushnumber(L, scalar);
         lua_arith     (L, LUA_OPDIV); // 4
         lua_getfield  (L, 1, "z");
         lua_pushnumber(L, scalar);
         lua_arith     (L, LUA_OPDIV); // 5
         //
         cls::push_new_instance(L, lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6));
         return 1;
      }
      luastackchange_t __sub(lua_State* L) { // creates and returns new vector
         return _simple_vector_operator_overload(L, LUA_OPSUB);
      }
      luastackchange_t __tostring(lua_State* L) { // creates and returns new vector
         lua_settop(L, 1);
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         lua_pushfstring(L, "(%f, %f, %f)", lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
         return 1;
      }
      luastackchange_t add(lua_State* L) { // modifies (self)
         return _simple_vector_assign_operator_overload(L, LUA_OPADD);
      }
      luastackchange_t div(lua_State* L) { // modifies (self)
         cls::require_self_type(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected non-zero number");
         auto scalar = lua_tonumber(L, 2);
         if (scalar == 0.0)
            luaL_error(L, "bad argument #%i (expected non-zero number; got zero)", 2);
         //
         lua_getfield  (L, 1, "x");
         lua_pushnumber(L, scalar);
         lua_arith     (L, LUA_OPDIV); // 3
         lua_setfield  (L, 1, "x");
         //
         lua_getfield  (L, 1, "y");
         lua_pushnumber(L, scalar);
         lua_arith     (L, LUA_OPDIV); // 4
         lua_setfield  (L, 1, "y");
         //
         lua_getfield  (L, 1, "z");
         lua_pushnumber(L, scalar);
         lua_arith     (L, LUA_OPDIV); // 5
         lua_setfield  (L, 1, "z");
         //
         lua_settop(L, 1); // return (self) to allow chaining
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
         cls::push_new_instance(L, x, y, z);
         return 1;
      }
      luastackchange_t cross(lua_State* L) { // compute the cross product vector of two 3D vectors
         cls::require_self_type(L);
         cls::require_arg_type(L, 2);
         lua_settop(L, 2);
         //
         lua_Number ax, ay, az;
         lua_Number bx, by, bz;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         ax = lua_tonumber(L, 3);
         ay = lua_tonumber(L, 4);
         az = lua_tonumber(L, 5);
         lua_settop(L, 2);
         lua_getfield(L, 2, "x");
         lua_getfield(L, 2, "y");
         lua_getfield(L, 2, "z");
         bx = lua_tonumber(L, 3);
         by = lua_tonumber(L, 4);
         bz = lua_tonumber(L, 5);
         lua_settop(L, 2);
         //
         lua_Number x = ay * bz - az * by;
         lua_Number y = az * bx - ax * bz;
         lua_Number z = ax * by - ay * bx;
         cls::push_new_instance(L, x, y, z);
         return 1;
      }
      luastackchange_t length(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         lua_checkstack(L, 4);
         //
         lua_Number x, y, z;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         x = lua_tonumber(L, 3);
         y = lua_tonumber(L, 4);
         z = lua_tonumber(L, 5);
         //
         lua_Number length = sqrt(x*x + y*y + z*z);
         lua_pushnumber(L, length);
         return 1;
      }
      luastackchange_t length_squared(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         lua_checkstack(L, 4);
         //
         lua_Number x, y, z;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         x = lua_tonumber(L, 3);
         y = lua_tonumber(L, 4);
         z = lua_tonumber(L, 5);
         //
         lua_Number length = x*x + y*y + z*z;
         lua_pushnumber(L, length);
         return 1;
      }
      luastackchange_t sub(lua_State* L) { // modifies (self)
         return _simple_vector_assign_operator_overload(L, LUA_OPSUB);
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_obj(lua_State* L) {
         //
         // function vector3:new(a, b, c)
         //    if tonumber(a) then
         //       return _make_vector(a, b, c)
         //    end
         //    if type(a) == "table" or type(a) == "userdata" then
         //       return _make_vector(a.x or 0, a.y or 0, a.z or 0)
         //    end
         //    return _make_vector(0, 0, 0)
         // end
         //
         if (lua_isnumber(L, 1)) {
            cls::push_new_instance(L, lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
            return 1;
         }
         auto rawtype = lua_type(L, 1);
         if (rawtype == LUA_TTABLE || rawtype == LUA_TUSERDATA) {
            lua_settop(L, 1);
            lua_getfield(L, 1, "x");
            lua_getfield(L, 1, "y");
            lua_getfield(L, 1, "z");
            cls::push_new_instance(L, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
            return 1;
         }
         cls::push_new_instance(L, 0.0, 0.0, 0.0);
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
   /*static*/ std::initializer_list<luaL_Reg> vector3::metatable_methods = {
      { "__add",          &_methods::__add }, // operator+
      { "__div",          &_methods::__div }, // operator/
      { "__sub",          &_methods::__sub }, // operator-
      { "__tostring",     &_methods::__tostring },
      { "add",            &_methods::add },   // operator+=
      { "div",            &_methods::div },   // operator/=
      { "copy",           &_methods::copy },
      { "cross",          &_methods::cross },
      { "length",         &_methods::length },
      { "length_squared", &_methods::length_squared },
      { "sub",            &_methods::sub },   // operator-=
   };

   /*static*/ void vector3::push_new_instance(lua_State* L, lua_Number x, lua_Number y, lua_Number z) {
      lua_createtable  (L, 0, 3);
      auto index = lua_gettop(L);
      luaL_getmetatable(L, metatable_key);
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

   /*static*/ void vector3::setup(lua_State* L) {
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