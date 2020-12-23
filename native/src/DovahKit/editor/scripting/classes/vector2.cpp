#include "vector2.h"
#include "../util.h"
#include <cmath>

namespace {
   using namespace editor_script;
   using cls = classes::vector2;

   void _make_vector(lua_State* L, double x, double y) { // creates a new vector2 (as a normal table with "x" and "y" members) on the top of the stack
      lua_createtable  (L, 0, 2);
      auto index = lua_gettop(L);
      luaL_getmetatable(L, cls::metatable_key);
      lua_setmetatable (L, index);
      lua_pushstring   (L, "x");
      lua_pushnumber   (L, x);
      lua_rawset       (L, index);
      lua_pushstring   (L, "y");
      lua_pushnumber   (L, y);
      lua_rawset       (L, index);
   }

   namespace _methods {
      luastackchange_t dot(lua_State* L);     // forward-declare so __mul can call it
      luastackchange_t length(lua_State* L);  // forward-declare so flatten can call it
      luastackchange_t project(lua_State* L); // forward-declare so flatten can call it
      //
      luastackchange_t __add(lua_State* L) { // creates and returns new vector
         cls::require_self_type(L);
         auto rawtype = lua_type(L, 2);
         if (rawtype != LUA_TTABLE && rawtype != LUA_TUSERDATA) {
            luaL_error(L, "bad argument #%i (expected table or userdata)", 2);
            __assume(0);
         }
         //
         lua_Number x, y;
         //
         lua_getfield(L, 1, "x");
         lua_getfield(L, 2, "x");
         x = lua_tonumber(L, 3) + lua_tonumber(L, 4);
         //
         lua_settop(L, 2);
         lua_getfield(L, 1, "y");
         lua_getfield(L, 2, "y");
         y = lua_tonumber(L, 3) + lua_tonumber(L, 4);
         //
         _make_vector(L, x, y);
         return 1;
      }
      luastackchange_t __div(lua_State* L) { // creates and returns new vector
         cls::require_self_type(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected non-zero number");
         auto scalar = lua_tonumber(L, 2);
         if (scalar == 0.0)
            luaL_error(L, "bad argument #%i (expected non-zero number; got zero)", 2);
         //
         lua_Number x, y;
         //
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         x = lua_tonumber(L, 3) / scalar;
         y = lua_tonumber(L, 4) / scalar;
         //
         _make_vector(L, x, y);
         return 1;
      }
      luastackchange_t __mul(lua_State* L) {
         cls::require_self_type(L);
         auto rawtype = lua_type(L, 2);
         if (rawtype == LUA_TNUMBER) {
            lua_Number scalar = lua_tonumber(L, 2);
            lua_Number x, y;
            lua_getfield(L, 1, "x");
            lua_getfield(L, 1, "y");
            _make_vector(L, lua_tonumber(L, 3) * scalar, lua_tonumber(L, 4) * scalar);
            return 1;
         }
         if (rawtype != LUA_TTABLE && rawtype != LUA_TUSERDATA) {
            luaL_error(L, "bad argument #%i (expected number, table, or userdata)", 2);
            __assume(0);
         }
         return dot(L);
      }
      luastackchange_t __sub(lua_State* L) { // creates and returns new vector
         cls::require_self_type(L);
         auto rawtype = lua_type(L, 2);
         if (rawtype != LUA_TTABLE && rawtype != LUA_TUSERDATA) {
            luaL_error(L, "bad argument #%i (expected table or userdata)", 2);
            __assume(0);
         }
         //
         lua_Number x, y;
         //
         lua_getfield(L, 1, "x");
         lua_getfield(L, 2, "x");
         x = lua_tonumber(L, 3) - lua_tonumber(L, 4);
         //
         lua_settop(L, 2);
         lua_getfield(L, 1, "y");
         lua_getfield(L, 2, "y");
         y = lua_tonumber(L, 3) - lua_tonumber(L, 4);
         //
         _make_vector(L, x, y);
         return 1;
      }
      luastackchange_t add(lua_State* L) { // modifies (self)
         cls::require_self_type(L);
         auto rawtype = lua_type(L, 2);
         if (rawtype != LUA_TTABLE && rawtype != LUA_TUSERDATA) {
            luaL_error(L, "bad argument #%i (expected table or userdata)", 2);
            __assume(0);
         }
         //
         lua_Number temp;
         //
         lua_settop(L, 2);
         lua_getfield(L, 1, "x");
         lua_getfield(L, 2, "x");
         temp = lua_tonumber(L, 3) + lua_tonumber(L, 4);
         lua_pushnumber(L, temp);
         lua_setfield  (L, 1, "x");
         //
         lua_settop(L, 2);
         lua_getfield(L, 1, "y");
         lua_getfield(L, 2, "y");
         temp = lua_tonumber(L, 3) + lua_tonumber(L, 4);
         lua_pushnumber(L, temp);
         lua_setfield  (L, 1, "y");
         //
         lua_settop(L, 1); // return (self) to allow chaining
         return 1;
      }
      luastackchange_t div(lua_State* L) { // modifies (self)
         cls::require_self_type(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected non-zero number");
         auto scalar = lua_tonumber(L, 2);
         if (scalar == 0.0)
            luaL_error(L, "bad argument #%i (expected non-zero number; got zero)", 2);
         //
         lua_Number temp;
         //
         lua_settop(L, 2);
         lua_getfield(L, 1, "x");
         temp = lua_tonumber(L, 3) / scalar;
         lua_pushnumber(L, temp);
         lua_setfield  (L, 1, "x");
         //
         lua_settop(L, 2);
         lua_getfield(L, 1, "y");
         temp = lua_tonumber(L, 3) / scalar;
         lua_pushnumber(L, temp);
         lua_setfield  (L, 1, "y");
         //
         lua_settop(L, 1); // return (self) to allow chaining
         return 1;
      }
      luastackchange_t copy(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         lua_Number x, y;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         x = lua_tonumber(L, 3);
         y = lua_tonumber(L, 4);
         //
         _make_vector(L, x, y);
         return 1;
      }
      luastackchange_t cross(lua_State* L) { // compute the cross product vector of two 2D vectors
         cls::require_self_type(L);
         cls::require_arg_type(L, 2);
         lua_settop(L, 2);
         //
         lua_checkstack(L, 5);
         lua_Number out;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 2, "y");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 2, "x");
         out = (lua_tonumber(L, 3) * lua_tonumber(L, 4)) - (lua_tonumber(L, 5) * lua_tonumber(L, 6));
         lua_pushnumber(L, out);
         return 1;
      }
      luastackchange_t dot(lua_State* L) { // compute the dot product vector of two 2D vectors
         cls::require_self_type(L);
         cls::require_arg_type(L, 2);
         lua_settop(L, 2);
         //
         lua_checkstack(L, 5);
         lua_Number out;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 2, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 2, "y");
         out = (lua_tonumber(L, 3) * lua_tonumber(L, 4)) + (lua_tonumber(L, 5) * lua_tonumber(L, 6));
         lua_pushnumber(L, out);
         return 1;
      }
      luastackchange_t flatten(lua_State* L) { // flatten a point into a one-dimensional distance from the origin
         cls::require_self_type(L);
         cls::require_arg_type(L, 2);
         lua_settop(L, 2);
         lua_checkstack(L, 5);
         //
         lua_pushcfunction(L, &project);
         lua_pushvalue(L, 1);
         lua_pushvalue(L, 2);
         lua_call(L, 2, 1);
         auto index_proj = 3; // STACK: - [ self, axis, projected ] +
         //
         lua_pushcfunction(L, &length);
         lua_pushvalue(L, index_proj);
         lua_call(L, 1, 1);
         lua_Number lng = lua_tonumber(L, 4); // STACK: - [ self, axis, projected, projected:length() ] +
         //
         lua_pushcfunction(L, &dot);
         lua_pushvalue(L, index_proj);
         lua_pushvalue(L, 2);
         lua_call(L, 2, 1);
         // STACK: - [ self, axis, projected, projected:length(), projected:dot(axis) ] +
         if (lua_tonumber(L, 5) < 0)
            lng = -lng;
         //
         lua_pushnumber(L, lng);
         return 1;
      }
      luastackchange_t length(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         lua_checkstack(L, 3);
         //
         lua_Number x, y, z;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         x = lua_tonumber(L, 3);
         y = lua_tonumber(L, 4);
         //
         lua_Number length = sqrt(x*x + y*y);
         lua_pushnumber(L, length);
         return 1;
      }
      luastackchange_t length_squared(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         lua_checkstack(L, 3);
         //
         lua_Number x, y, z;
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         x = lua_tonumber(L, 3);
         y = lua_tonumber(L, 4);
         //
         lua_Number length = x*x + y*y;
         lua_pushnumber(L, length);
         return 1;
      }
      luastackchange_t normal(lua_State* L) {
         cls::require_self_type(L);
         bool righthand = lua_toboolean(L, 2);
         lua_settop(L, 1);
         //
         lua_getfield(L, 1, "x"); // 2
         lua_getfield(L, 1, "y"); // 3
         //
         lua_Number x, y;
         if (righthand) {
            x = -lua_tonumber(L, 3);
            y =  lua_tonumber(L, 2);
         } else {
            x =  lua_tonumber(L, 3);
            y = -lua_tonumber(L, 2);
         }
         _make_vector(L, x, y);
         return 1;
      }
      luastackchange_t project(lua_State* L) {
         cls::require_self_type(L);
         cls::require_arg_type(L, 2);
         lua_settop(L, 2);
         lua_checkstack(L, 5);
         //
         lua_pushcfunction(L, &dot);
         lua_pushvalue(L, 1);
         lua_pushvalue(L, 2);
         lua_call(L, 2, 1);
         //
         lua_pushcfunction(L, &dot);
         lua_pushvalue(L, 2);
         lua_pushvalue(L, 2);
         lua_call(L, 2, 1);
         //
         lua_Number dot_prod = lua_tonumber(L, 3);
         lua_Number b_len_sq = lua_tonumber(L, 4);
         lua_Number quotient = dot_prod / b_len_sq;
         //
         lua_getfield(L, 2, "x");
         lua_getfield(L, 2, "y");
         _make_vector(L, quotient * lua_tonumber(L, 5), quotient * lua_tonumber(L, 6));
         return 1;
      }
      luastackchange_t rotate(lua_State* L) {
         cls::require_self_type(L);
         auto degrees = lua_tonumber(L, 2);
         lua_settop(L, 1);
         //
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_Number x = lua_tonumber(L, 2);
         lua_Number y = lua_tonumber(L, 2);
         //
         lua_Number cd = cos(degrees);
         lua_Number sd = sin(degrees);
         x = cd * x - sd * y;
         y = sd * x + cd * y;
         _make_vector(L, x, y);
         return 1;
      }
      luastackchange_t sub(lua_State* L) { // modifies (self)
         cls::require_self_type(L);
         auto rawtype = lua_type(L, 2);
         if (rawtype != LUA_TTABLE && rawtype != LUA_TUSERDATA) {
            luaL_error(L, "bad argument #%i (expected table or userdata)", 2);
            __assume(0);
         }
         //
         lua_Number temp;
         //
         lua_settop(L, 2);
         lua_getfield(L, 1, "x");
         lua_getfield(L, 2, "x");
         temp = lua_tonumber(L, 3) - lua_tonumber(L, 4);
         lua_pushnumber(L, temp);
         lua_setfield  (L, 1, "x");
         //
         lua_settop(L, 2);
         lua_getfield(L, 1, "y");
         lua_getfield(L, 2, "y");
         temp = lua_tonumber(L, 3) - lua_tonumber(L, 4);
         lua_pushnumber(L, temp);
         lua_setfield  (L, 1, "y");
         //
         lua_settop(L, 1); // return (self) to allow chaining
         return 1;
      }
   }
}
namespace editor_script::classes {
   /*static*/ luaL_Reg vector2::metatable_methods[] = {
      { "__add",          &_methods::__add }, // operator+
      { "__div",          &_methods::__div }, // operator/
      { "__mul",          &_methods::__mul }, // operator*
      { "__sub",          &_methods::__sub }, // operator-
      { "add",            &_methods::add },   // operator+=
      { "div",            &_methods::div },   // operator/=
      { "dot",            &_methods::dot },
      { "copy",           &_methods::copy },
      { "cross",          &_methods::cross },
      { "flatten",        &_methods::flatten },
      { "length",         &_methods::length },
      { "length_squared", &_methods::length_squared },
      { "normal",         &_methods::normal },
      { "rotate",         &_methods::rotate },
      { "project",        &_methods::project },
      { "sub",            &_methods::sub },   // operator-=
      { nullptr, nullptr },
   };
}