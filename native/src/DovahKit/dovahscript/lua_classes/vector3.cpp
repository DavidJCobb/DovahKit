#include "vector3.h"
#include <array>
#include <cmath>
#include "../../helpers/lua/error.h"
#include "../../helpers/lua/is_invocable.h"

namespace {
   using namespace dovahscript;
   using cls = lua_classes::vector3;

   constexpr const std::array axis_names = { "x", "y", "z" };

   int _simple_vector_operator_overload(lua_State* L, int op) { // returns new vector
      cls::require_self_type(L);
      auto rawtype = lua_type(L, 2);
      if (rawtype != LUA_TTABLE && rawtype != LUA_TUSERDATA) {
         cobb::lua::argerror(L, 2, "expected table or userdata");
      }
      lua_settop(L, 2);
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
         cobb::lua::argerror(L, 2, "expected table or userdata");
      }
      lua_settop(L, 2);
      lua_getfield(L, 1, "set_xyz");
      if (cobb::lua::is_invocable(L, -1)) {
         for (const auto* name : axis_names) {
            lua_getfield(L, 1, name);
            lua_getfield(L, 2, name);
            lua_arith(L, op);
         }
         lua_call(L, 4, 0);
         return 0;
      } else {
         //
         // A subclass overrode (set_xyz) with an unusable value. Fall back to 
         // setting fields one by one.
         //
         lua_pop(L, 1);
         for (const auto* name : axis_names) {
            lua_getfield(L, 1, name);
            lua_getfield(L, 2, name);
            lua_arith(L, op);
            lua_setfield(L, 1, name);
         }
      }
      return 0;
   }

   namespace _methods {
      int __add(lua_State* L) { // creates and returns new vector
         return _simple_vector_operator_overload(L, LUA_OPADD);
      }
      int __div(lua_State* L) { // creates and returns new vector
         cls::require_self_type(L);
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "expected non-zero number");
         auto scalar = lua_tonumber(L, 2);
         cobb::lua::argcheck(L, scalar != 0.0, 2, "expected non-zero divisor; got zero");
         lua_settop(L, 1);
         //
         for (const auto* name : axis_names) {
            lua_getfield  (L, 1, name);   // 3...
            lua_pushnumber(L, scalar);    // 4...
            lua_arith     (L, LUA_OPDIV); // 3...
         }
         //
         cls::push_new_instance(L, lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
         return 1;
      }
      int __sub(lua_State* L) { // creates and returns new vector
         return _simple_vector_operator_overload(L, LUA_OPSUB);
      }
      int __tostring(lua_State* L) { // creates and returns new vector
         lua_settop(L, 1);
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         lua_pushfstring(L, "(%f, %f, %f)", lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
         return 1;
      }
      int add(lua_State* L) { // modifies (self)
         return _simple_vector_assign_operator_overload(L, LUA_OPADD);
      }
      int div(lua_State* L) { // modifies (self)
         cls::require_self_type(L);
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "expected non-zero number");
         auto scalar = lua_tonumber(L, 2);
         cobb::lua::argcheck(L, scalar != 0.0, 2, "expected non-zero divisor; got zero");
         //
         lua_settop(L, 1);
         lua_getfield(L, 1, "set_xyz");
         if (cobb::lua::is_invocable(L, -1)) {
            for (const auto* name : axis_names) {
               lua_getfield  (L, 1, name);
               lua_pushnumber(L, scalar);
               lua_arith     (L, LUA_OPDIV);
            }
            lua_call(L, 4, 0);
            return 0;
         } else {
            //
            // A subclass overrode (set_xyz) with an unusable value. Fall back to 
            // setting fields one by one.
            //
            lua_pop(L, 1);
            for (const auto* name : axis_names) {
               lua_getfield  (L, 1, name);
               lua_pushnumber(L, scalar);
               lua_arith     (L, LUA_OPDIV);
               lua_setfield  (L, 1, name);
            }
         }
         return 0;
      }
      int copy(lua_State* L) {
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
      int cross(lua_State* L) { // compute the cross product vector of two 3D vectors
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
      int length(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         lua_checkstack(L, 4);
         //
         lua_Number x, y, z;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         x = lua_tonumber(L, 2);
         y = lua_tonumber(L, 3);
         z = lua_tonumber(L, 4);
         //
         lua_Number length = sqrt(x*x + y*y + z*z);
         lua_pushnumber(L, length);
         return 1;
      }
      int length_squared(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         lua_checkstack(L, 4);
         //
         lua_Number x, y, z;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         x = lua_tonumber(L, 2);
         y = lua_tonumber(L, 3);
         z = lua_tonumber(L, 4);
         //
         lua_Number length = x*x + y*y + z*z;
         lua_pushnumber(L, length);
         return 1;
      }
      int normalize(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         lua_checkstack(L, 4);
         //
         lua_Number x, y, z;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         x = lua_tonumber(L, 2);
         y = lua_tonumber(L, 3);
         z = lua_tonumber(L, 4);
         lua_Number length = sqrt(x * x + y * y + z * z);
         //
         lua_settop(L, 2);
         lua_getfield(L, 1, "set_xyz");
         if (cobb::lua::is_invocable(L, -1)) {
            lua_pushnumber(L, x / length);
            lua_pushnumber(L, y / length);
            lua_pushnumber(L, z / length);
            lua_call(L, 4, 0);
            return 0;
         } else {
            //
            // A subclass overrode (set_xyz) with an unusable value. Fall back to 
            // setting fields one by one.
            //
            lua_pop(L, 1);
            lua_pushnumber(L, x / length);
            lua_setfield(L, 1, "x");
            lua_pushnumber(L, y / length);
            lua_setfield(L, 1, "y");
            lua_pushnumber(L, z / length);
            lua_setfield(L, 1, "z");
         }
         return 0;
      }
      int normalized(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         lua_checkstack(L, 4);
         //
         lua_Number x, y, z;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         x = lua_tonumber(L, 2);
         y = lua_tonumber(L, 3);
         z = lua_tonumber(L, 4);
         //
         lua_Number length = sqrt(x * x + y * y + z * z);
         cls::push_new_instance(L, x / length, y / length, z / length);
         return 1;
      }
      int set_xyz(lua_State* L) {
         //
         // Default implementation. Subclasses, particularly native code, could replace 
         // this function in order to more efficiently handle overwriting an entire 
         // vector3 instance.
         //
         cls::require_self_type(L);
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number (x) expected");
         cobb::lua::argcheck(L, lua_isnumber(L, 3), 3, "number (y) expected");
         cobb::lua::argcheck(L, lua_isnumber(L, 4), 4, "number (z) expected");
         lua_settop(L, 4);
         lua_setfield(L, 1, "z");
         lua_setfield(L, 1, "y");
         lua_setfield(L, 1, "x");
         return 0;
      }
      int sub(lua_State* L) { // modifies (self)
         return _simple_vector_assign_operator_overload(L, LUA_OPSUB);
      }
   }

   namespace _singleton_functions {
      int new_obj(lua_State* L) {
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
      int is(lua_State* L) {
         if (cls::check_arg_type(L, 1)) {
            lua_pushboolean(L, 1);
            return 1;
         }
         lua_pushboolean(L, 0);
         return 1;
      }
   }
}
namespace dovahscript::lua_classes {
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_methods = {
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
      { "normalize",      &_methods::normalize },  // modify the vector in-place
      { "normalized",     &_methods::normalized }, // return a normalized copy
      { "set_xyz",        &_methods::set_xyz },
      { "sub",            &_methods::sub },   // operator-=
   };

   /*static*/ void cls::push_new_instance(lua_State* L, lua_Number x, lua_Number y, lua_Number z) {
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

   /*static*/ void cls::setup(lua_State* L) {
      dovahscript::classes::define_class(L, metatable_key, nullptr, metatable_methods);
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