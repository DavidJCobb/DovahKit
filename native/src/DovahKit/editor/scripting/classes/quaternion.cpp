#include "quaternion.h"
#include "../util.h"
#include <cmath>

#include "../../../helpers/rotation.h"
#include "../../../helpers/lua/set_top_on_exit.h"
#include "euler.h"
#include "matrix3x3.h"

namespace {
   using namespace editor_script;
   using cls = classes::quaternion;

   void _require_similar_argument(lua_State* L, int stack_pos) {
      auto rawtype = lua_type(L, stack_pos);
      if (rawtype != LUA_TTABLE && rawtype != LUA_TUSERDATA)
         luaL_error(L, "bad argument #%i (expected quaternion-like table or userdata)", stack_pos);
      if (cls::check_arg_type(L, stack_pos))
         return;
      //
      auto prior = lua_gettop(L);
      auto guard = cobb::lua::set_top_on_exit(L, prior);
      constexpr const char* warning_text = "quaternion operator overload given a non-quaternion-like table operand";
      //
      lua_getfield(L, stack_pos, "w");
      if (!lua_isnumber(L, -1)) {
         lua_warning(L, warning_text, 0);
         return;
      }
      lua_getfield(L, stack_pos, "x");
      if (!lua_isnumber(L, -1)) {
         lua_warning(L, warning_text, 0);
         return;
      }
      lua_getfield(L, stack_pos, "y");
      if (!lua_isnumber(L, -1)) {
         lua_warning(L, warning_text, 0);
         return;
      }
      lua_getfield(L, stack_pos, "z");
      if (!lua_isnumber(L, -1)) {
         lua_warning(L, warning_text, 0);
      }
   }

   int _scalar_operator_overload(lua_State* L, int op) { // returns new vector
      cls::require_self_type(L);
      luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected number");
      lua_settop(L, 2);
      //
      lua_getfield (L, 1, "w");
      lua_pushvalue(L, 2);
      lua_arith(L, op);
      //
      lua_getfield (L, 1, "x");
      lua_pushvalue(L, 2);
      lua_arith(L, op);
      //
      lua_getfield (L, 1, "y");
      lua_pushvalue(L, 2);
      lua_arith(L, op);
      //
      lua_getfield (L, 1, "z");
      lua_pushvalue(L, 2);
      lua_arith(L, op);
      //
      cls::push_new_instance(L, { lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6) });
      return 1;
   }
   int _scalar_assign_operator_overload(lua_State* L, int op) { // returns new vector
      cls::require_self_type(L);
      luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected number");
      lua_settop(L, 2);
      //
      lua_getfield(L, 1, "w");
      lua_pushvalue(L, 2);
      lua_arith(L, op);
      lua_setfield(L, 1, "w");
      //
      lua_getfield(L, 1, "x");
      lua_pushvalue(L, 2);
      lua_arith(L, op);
      lua_setfield(L, 1, "x");
      //
      lua_getfield(L, 1, "y");
      lua_pushvalue(L, 2);
      lua_arith(L, op);
      lua_setfield(L, 1, "y");
      //
      lua_getfield(L, 1, "z");
      lua_pushvalue(L, 2);
      lua_arith(L, op);
      lua_setfield(L, 1, "z");
      //
      lua_settop(L, 1); // return (self) to allow chaining
      return 1;
   }

   int _simple_operator_overload(lua_State* L, int op) { // returns new vector
      cls::require_self_type(L);
      _require_similar_argument(L, 2);
      lua_settop(L, 2);
      //
      lua_getfield(L, 1, "w");
      lua_getfield(L, 2, "w");
      lua_arith(L, op);
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
      cls::push_new_instance(L, { lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6) });
      return 1;
   }
   int _simple_assign_operator_overload(lua_State* L, int op) { // modifies and returns self
      cls::require_self_type(L);
      _require_similar_argument(L, 2);
      lua_settop(L, 2);
      //
      lua_getfield(L, 1, "w");
      lua_getfield(L, 2, "w");
      lua_arith(L, op);
      lua_setfield(L, 1, "w");
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
         return _simple_operator_overload(L, LUA_OPADD);
      }
      luastackchange_t __div(lua_State* L) { // creates and returns new vector
         return _scalar_operator_overload(L, LUA_OPDIV);
      }
      luastackchange_t __mul(lua_State* L) { // creates and returns new vector
         cls::require_self_type(L);
         if (lua_isnumber(L, 2))
            return _simple_operator_overload(L, LUA_OPMUL);
         //
         _require_similar_argument(L, 2);
         auto a = cls::extract_from_stack(L, 1);
         auto b = cls::extract_from_stack(L, 2);
         a *= b;
         cls::push_new_instance(L, a);
         return 1;
      }
      luastackchange_t __sub(lua_State* L) { // creates and returns new vector
         return _simple_operator_overload(L, LUA_OPSUB);
      }
      luastackchange_t __tostring(lua_State* L) { // creates and returns new vector
         lua_settop(L, 1);
         auto raw = cls::extract_from_stack(L, 1);
         lua_pushfstring(L, "(%f, %f, %f, %f)", raw.w, raw.x, raw.y, raw.z);
         return 1;
      }
      luastackchange_t add(lua_State* L) { // modifies (self)
         return _simple_assign_operator_overload(L, LUA_OPADD);
      }
      luastackchange_t conjugate(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         auto raw = cls::extract_from_stack(L, 1).conjugate();
         cls::push_new_instance(L, raw);
         return 1;
      }
      luastackchange_t copy(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         cls::push_new_instance(L, cls::extract_from_stack(L, 1));
         return 1;
      }
      luastackchange_t div(lua_State* L) { // modifies (self)
         return _scalar_assign_operator_overload(L, LUA_OPDIV);
      }
      luastackchange_t inverse(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         auto raw = cls::extract_from_stack(L, 1).inverse();
         cls::push_new_instance(L, raw);
         return 1;
      }
      luastackchange_t mul(lua_State* L) { // modifies (self)
         cls::require_self_type(L);
         if (lua_isnumber(L, 2))
            return _simple_assign_operator_overload(L, LUA_OPMUL);
         //
         _require_similar_argument(L, 2);
         auto a = cls::extract_from_stack(L, 1);
         auto b = cls::extract_from_stack(L, 2);
         a *= b;
         lua_pushnumber(L, a.w);
         lua_setfield  (L, 1, "w");
         lua_pushnumber(L, a.x);
         lua_setfield  (L, 1, "x");
         lua_pushnumber(L, a.y);
         lua_setfield  (L, 1, "y");
         lua_pushnumber(L, a.z);
         lua_setfield  (L, 1, "z");
         lua_settop(L, 1);
         return 1;
      }
      luastackchange_t norm(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         auto raw = cls::extract_from_stack(L, 1);
         lua_pushnumber(L, raw.norm());
         return 1;
      }
      luastackchange_t sub(lua_State* L) { // modifies (self)
         return _simple_assign_operator_overload(L, LUA_OPSUB);
      }
      luastackchange_t to_euler(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         auto converted = (cobb::euler) cls::extract_from_stack(L, 1);
         classes::euler::push_new_instance(L, converted);
         return 1;
      }
      luastackchange_t to_matrix(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         auto converted = (cobb::rotation_matrix) cls::extract_from_stack(L, 1);
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
   /*static*/ std::initializer_list<luaL_Reg> quaternion::metatable_methods = {
      { "__add",      &_methods::__add },
      { "__div",      &_methods::__div },
      { "__mul",      &_methods::__mul },
      { "__sub",      &_methods::__sub },
      { "__tostring", &_methods::__tostring },
      { "add",        &_methods::add },
      { "conjugate",  &_methods::conjugate },
      { "copy",       &_methods::copy },
      { "div",        &_methods::div },
      { "inverse",    &_methods::inverse },
      { "mul",        &_methods::mul },
      { "norm",       &_methods::norm },
      { "sub",        &_methods::sub },
      { "to_euler",   &_methods::to_euler },
      { "to_matrix",  &_methods::to_matrix },
   };

   /*static*/ cobb::quaternion quaternion::extract_from_stack(lua_State* L, int stack_pos) {
      cobb::quaternion raw;
      auto prior = lua_gettop(L);
      lua_getfield(L, stack_pos, "w");
      lua_getfield(L, stack_pos, "x");
      lua_getfield(L, stack_pos, "y");
      lua_getfield(L, stack_pos, "z");
      raw.w = lua_tonumber(L, prior + 1);
      raw.x = lua_tonumber(L, prior + 2);
      raw.y = lua_tonumber(L, prior + 3);
      raw.z = lua_tonumber(L, prior + 4);
      lua_settop(L, prior);
      return raw;
   }
   /*static*/ void quaternion::push_new_instance(lua_State* L, const cobb::quaternion& raw) {
      lua_createtable  (L, 0, 3);
      auto index = lua_gettop(L);
      luaL_getmetatable(L, cls::metatable_key);
      lua_setmetatable (L, index);
      lua_pushstring   (L, "w");
      lua_pushnumber   (L, raw.w);
      lua_rawset       (L, index);
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

   /*static*/ void quaternion::setup(lua_State* L) {
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