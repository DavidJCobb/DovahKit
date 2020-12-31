#include "euler.h"
#include "../util.h"
#include <cmath>

#include "../../../helpers/rotation.h"
#include "matrix3x3.h"
#include "quaternion.h"

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
      luastackchange_t to_quaternion(lua_State* L) { // compute the cross product vector of two 3D vectors
         cls::require_self_type(L);
         lua_settop(L, 1);
         //
         lua_getfield(L, 1, "x");
         lua_getfield(L, 1, "y");
         lua_getfield(L, 1, "z");
         auto converted = (cobb::quaternion) cobb::euler{ lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) };
         lua_settop(L, 1);
         //
         classes::quaternion::push_new_instance(L, converted);
         return 1;
      }
   }

   namespace _singleton_functions {
      luastackchange_t _new_obj(lua_State* L, bool degrees) {
         //
         // function _new_obj(a, b, c)
         //    if tonumber(a) then
         //       return _make_instance(a, b, c)
         //    end
         //    if type(a) == "table" or type(a) == "userdata" then
         //       return _make_instance(a.x or 0, a.y or 0, a.z or 0)
         //    end
         //    return _make_instance(0, 0, 0)
         // end
         //
         // -- _new_obj(1, 2, 3)
         // -- _new_obj({ 1, 2, 3 })
         // -- _new_obj({ x = 1, y = 2, z = 3 })
         // -- _new_obj({ 1, 2, z = 3})
         // -- _new_obj({ 1, 2 })        -- warns and uses zeroes for missing values
         // -- _new_obj({ foo = "bar" }) -- warns and uses zeroes for missing values
         // -- _new_obj()
         //
         if (lua_isnumber(L, 1)) {
            double coords[3];
            for (int i = 0; i < 3; ++i) {
               coords[i] = lua_tonumber(L, 1 + i);
               if (degrees)
                  coords[i] = cobb::degrees_to_radians(coords[i]);
            }
            cls::push_new_instance(L, { coords[0], coords[1], coords[2] });
            return 1;
         }
         auto rawtype = lua_type(L, 1);
         if (rawtype == LUA_TTABLE || rawtype == LUA_TUSERDATA) {
            lua_settop(L, 1);
            //
            double coords[3];
            char   s[2]   = "x";
            bool   warned = false;
            for (s[0] = 'x'; s[0] <= 'z'; ++s[0]) {
               int i = s[0] - 'x';
               //
               lua_getfield(L, 1, s);
               if (!lua_isnumber(L, 2)) {
                  lua_settop(L, 1);
                  lua_geti  (L, 1, i + 1);
                  if (!warned && !lua_isnumber(L, 2)) {
                     lua_warning(L, "attempting to construct an euler from a table unsuited to the task (non-numeric or absent array or x/y/z members)", 0);
                     warned = true;
                  }
               }
               coords[i] = lua_tonumber(L, 2);
               if (degrees)
                  coords[i] = cobb::degrees_to_radians(coords[i]);
               lua_settop(L, 1);
            }
            //
            cls::push_new_instance(L, { coords[0], coords[1], coords[2] });
            return 1;
         }
         cls::push_new_instance(L, {});
         return 1;
      }
      //
      luastackchange_t from_degrees(lua_State* L) {
         return _new_obj(L, true);
      }
      luastackchange_t from_radians(lua_State* L) {
         return _new_obj(L, false);
      }
      luastackchange_t new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            luaL_error(L, "the euler.new function should not be called with a colon or passed any arguments; use euler.from_degrees or euler.from_radians to initialize an euler with non-zero values");
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
      { "__tostring",    &_methods::__tostring },
      { "copy",          &_methods::copy },
      { "to_matrix",     &_methods::to_matrix },
      { "to_quaternion", &_methods::to_quaternion },
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
      lua_pushcfunction(L, &_singleton_functions::from_degrees);
      lua_setfield     (L, -2, "from_degrees");
      lua_pushcfunction(L, &_singleton_functions::from_radians);
      lua_setfield     (L, -2, "from_radians");
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      lua_setglobal(L, cls::global_name);
   }
}