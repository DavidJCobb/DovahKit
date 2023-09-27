#include "matrix3x3.h"
#include <cmath>
#include "../../helpers/lua/error.h"
#include "../../helpers/rotation.h"
#include "euler.h"
#include "quaternion.h"
#include "vector3.h"

namespace {
   using namespace dovahscript;
   using cls = lua_classes::matrix3x3;

   namespace _singleton_functions {
      int construct_from_extrinsic_zyx(lua_State* L) {
         luaL_argcheck(L, lua_isnumber(L, 1), 1, "expected X radians (number)");
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected Y radians (number)");
         luaL_argcheck(L, lua_isnumber(L, 3), 3, "expected Z radians (number)");
         double x = lua_tonumber(L, 1);
         double y = lua_tonumber(L, 2);
         double z = lua_tonumber(L, 3);
         bool   righthand = lua_toboolean(L, 4);
         cls::push_new_instance(L, cobb::rotation_matrix::construct_from_extrinsic_zyx(x, y, z, righthand));
         return 1;
      }
      int construct_from_x(lua_State* L) {
         luaL_argcheck(L, lua_isnumber(L, 1), 1, "expected radians (number)");
         double radians   = lua_tonumber(L, 1);
         bool   righthand = lua_toboolean(L, 2);
         cls::push_new_instance(L, cobb::rotation_matrix::construct_from_x(radians, righthand));
         return 1;
      }
      int construct_from_y(lua_State* L) {
         luaL_argcheck(L, lua_isnumber(L, 1), 1, "expected radians (number)");
         double radians   = lua_tonumber(L, 1);
         bool   righthand = lua_toboolean(L, 2);
         cls::push_new_instance(L, cobb::rotation_matrix::construct_from_y(radians, righthand));
         return 1;
      }
      int construct_from_z(lua_State* L) {
         luaL_argcheck(L, lua_isnumber(L, 1), 1, "expected radians (number)");
         double radians   = lua_tonumber(L, 1);
         bool   righthand = lua_toboolean(L, 2);
         cls::push_new_instance(L, cobb::rotation_matrix::construct_from_z(radians, righthand));
         return 1;
      }
      int new_obj(lua_State* L) {
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
         cobb::rotation_matrix raw;
         if (lua_isnumber(L, 1)) {
            for (int i = 0; i < 9; ++i)
               raw.at(i) = lua_tonumber(L, i + 2);
            cls::push_new_instance(L, raw);
            return 1;
         }
         auto rawtype = lua_type(L, 1);
         if (rawtype == LUA_TTABLE || rawtype == LUA_TUSERDATA) {
            lua_settop(L, 1);
            //
            lua_geti(L, 1, 1);
            rawtype = lua_type(L, 2);
            lua_settop(L, 1);
            if (rawtype == LUA_TTABLE) {
               //
               // Two-dimensional array (matrix[y][x])
               //
               for (int y = 0; y < 3; ++y) {
                  lua_geti(L, 1, y + 1);
                  for (int x = 0; x < 3; ++x) {
                     lua_geti(L, 2, x + 1);
                     raw[y][x] = lua_tonumber(L, 3);
                     lua_settop(L, 2);
                  }
                  lua_settop(L, 1);
               }
            } else {
               //
               // Flat array.
               //
               for (int i = 0; i < 9; ++i) {
                  lua_geti(L, 1, i + 1);
                  raw.at(i) = lua_tonumber(L, 2);
                  lua_settop(L, 1);
               }
            }
            //
            cls::push_new_instance(L, raw);
            return 1;
         }
         for (int y = 0; y < decltype(raw)::height; ++y)
            for (int x = 0; x < decltype(raw)::width; ++x)
               raw.rows[y][x] = 0;
         cls::push_new_instance(L, {});
         return 1;
      }
      int identity(lua_State* L) {
         cls::push_new_instance(L, cobb::rotation_matrix::identity());
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

   namespace _methods {
      int __mul(lua_State* L) {
         cls::require_self_type(L);
         auto rawtype = lua_type(L, 2);
         if (rawtype == LUA_TNUMBER) {
            auto raw    = cls::extract_from_stack(L, 1);
            auto scalar = lua_tonumber(L, 2);
            raw *= scalar;
            cls::push_new_instance(L, raw);
            return 1;
         }
         if (rawtype == LUA_TTABLE || rawtype == LUA_TUSERDATA) {
            auto a = cls::extract_from_stack(L, 1);
            if (lua_classes::vector3::check_arg_type(L, 2)) {
               std::array<double, 3> v;
               lua_getfield(L, 2, "x");
               lua_getfield(L, 2, "y");
               lua_getfield(L, 2, "z");
               v[0] = lua_tonumber(L, 3);
               v[1] = lua_tonumber(L, 4);
               v[2] = lua_tonumber(L, 5);
               lua_settop(L, 0);
               v = a * v;
               lua_classes::vector3::push_new_instance(L, v[0], v[1], v[2]);
               return 1;
            }
            if (lua_classes::matrix3x3::check_arg_type(L, 2)) {
               auto b = cls::extract_from_stack(L, 3);
               a *= b;
               cls::push_new_instance(L, a);
               return 1;
            }
         }
         return luaL_error(L, "righthand operand (%s) is ambiguous; pass a number or an explicit matrix3x3 or vector3 instance", lua_typename(L, rawtype));
      }
      int copy(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         cls::push_new_instance(L, cls::extract_from_stack(L, 1));
         return 1;
      }
      int determinant(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         auto mat = cls::extract_from_stack(L, 1);
         lua_pushnumber(L, mat.determinant());
         return 1;
      }
      int mul(lua_State* L) {
         lua_settop(L, 2);
         //
         cls::require_self_type(L);
         auto rawtype = lua_type(L, 2);
         if (rawtype == LUA_TNUMBER) {
            auto scalar = lua_tonumber(L, 2);
            for (int i = 0; i < 9; ++i) {
               lua_geti      (L, 1, i + 1);
               lua_pushnumber(L, scalar);
               lua_arith     (L, LUA_OPMUL);
               lua_seti(L, 1, i + 1);
            }
            lua_settop(L, 1);
            return 1;
         }
         if (rawtype == LUA_TTABLE || rawtype == LUA_TUSERDATA) {
            auto a = cls::extract_from_stack(L, 1);
            if (lua_classes::vector3::check_arg_type(L, 2)) {
               std::array<double, 3> v;
               lua_getfield(L, 2, "x");
               lua_getfield(L, 2, "y");
               lua_getfield(L, 2, "z");
               v[0] = lua_tonumber(L, 3);
               v[1] = lua_tonumber(L, 4);
               v[2] = lua_tonumber(L, 5);
               lua_settop(L, 0);
               v = a * v;
               lua_classes::vector3::push_new_instance(L, v[0], v[1], v[2]);
               return 1;
            }
            if (lua_classes::matrix3x3::check_arg_type(L, 2)) {
               auto b = cls::extract_from_stack(L, 3);
               a *= b;
               lua_settop(L, 1);
               for (int i = 0; i < 9; ++i) {
                  lua_pushnumber(L, a.at(i));
                  lua_seti(L, 1, i + 1);
               }
               return 1;
            }
         }
         cobb::lua::error(L, "righthand operand (%s) is ambiguous; pass a number or an explicit matrix3x3 or vector3 instance", lua_typename(L, rawtype));
      }
      int to_euler(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         auto converted = (cobb::euler) cls::extract_from_stack(L, 1);
         lua_classes::euler::push_new_instance(L, converted);
         return 1;
      }
      int to_quaternion(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         auto converted = (cobb::quaternion) cls::extract_from_stack(L, 1);
         lua_classes::quaternion::push_new_instance(L, converted);
         return 1;
      }
      int set_row(lua_State* L) {
         cls::require_self_type(L);
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "integer expected");
         if (!lua_istable(L, 3) && !lua_isuserdata(L, 3)) {
            luaL_argcheck(L, false, 3, "table or userdata expected");
         }
         lua_settop(L, 3);
         //
         int row = lua_tointeger(L, 2);
         //
         for (int i = 0; i < 3; ++i) {
            lua_geti(L, 3, i + 1);
            if (lua_type(L, -1) != LUA_TNUMBER) { // force to a number
               auto num = lua_tonumber(L, -1);
               lua_pop(L, 1);
               lua_pushnumber(L, num);
            }
            lua_seti(L, 1, (row - 1) * 3 + i + 1);
         }
         return 0;
      }
      int trace(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         auto mat = cls::extract_from_stack(L, 1);
         lua_pushnumber(L, mat.trace());
         return 1;
      }
      int transpose(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         auto m = cls::extract_from_stack(L, 1);
         m.transpose();
         cls::push_new_instance(L, m);
         return 1;
      }
      int transpose_in_place(lua_State* L) {
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         auto m = cls::extract_from_stack(L, 1);
         m.transpose();
         for (int i = 0; i < 9; ++i) {
            lua_pushnumber(L, m.at(i));
            lua_seti(L, 1, i + 1);
         }
         return 1;
      }
   }
}
namespace dovahscript::lua_classes {
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "__mul",              &_methods::__mul }, // creates new instance; does not modify self
      { "copy",               &_methods::copy },
      { "determinant",        &_methods::determinant },
      { "mul",                &_methods::mul }, // modifies self (unless operand is not a matrix3x3)
      { "set_row",            &_methods::set_row },
      { "to_euler",           &_methods::to_euler },
      { "to_quaternion",      &_methods::to_quaternion },
      { "trace",              &_methods::trace },
      { "transpose",          &_methods::transpose },
      { "transpose_in_place", &_methods::transpose_in_place },
   };

   /*static*/ cobb::rotation_matrix cls::extract_from_stack(lua_State* L, int stack_pos) {
      cobb::rotation_matrix raw;
      auto prior = lua_gettop(L);
      for (int i = 0; i < 9; ++i) {
         lua_geti(L, stack_pos, i + 1);
         raw.at(i) = lua_tonumber(L, prior + 1);
         lua_settop(L, prior);
      }
      return raw;
   }
   /*static*/ void cls::push_new_instance(lua_State* L, const cobb::rotation_matrix& raw) {
      lua_createtable  (L, 0, 3);
      auto index = lua_gettop(L);
      luaL_getmetatable(L, cls::metatable_key);
      lua_setmetatable (L, index);
      for (int i = 0; i < 9; ++i) {
         lua_pushnumber(L, raw.at(i));
         lua_rawseti   (L, index, i + 1);
      }
   }

   /*static*/ void cls::setup(lua_State* L) {
      classes::define_class(L, metatable_key, nullptr, metatable_methods);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      auto index = lua_gettop(L);
      lua_pushcfunction(L, &_singleton_functions::construct_from_extrinsic_zyx);
      lua_setfield     (L, index, "construct_from_extrinsic_zyx");
      lua_pushcfunction(L, &_singleton_functions::construct_from_x);
      lua_setfield     (L, index, "construct_from_x");
      lua_pushcfunction(L, &_singleton_functions::construct_from_y);
      lua_setfield     (L, index, "construct_from_y");
      lua_pushcfunction(L, &_singleton_functions::construct_from_z);
      lua_setfield     (L, index, "construct_from_z");
      lua_pushcfunction(L, &_singleton_functions::new_obj);
      lua_setfield     (L, index, "new");
      lua_pushcfunction(L, &_singleton_functions::identity);
      lua_setfield     (L, index, "identity");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, index, "is");
      lua_setglobal(L, cls::global_name);
   }
}