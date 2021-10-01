#include "pull_rotation.h"
#include "../../helpers/lua/istablelike.h"
#include "../lua_classes/euler.h"
#include "../lua_classes/matrix3x3.h"
#include "../lua_classes/quaternion.h"

namespace {
   enum class _type {
      unknown,
      euler,
      matrix,
      quaternion,
   };

   _type _get_table_type(lua_State* L, int index) {
      using namespace dovahscript;
      //
      if (classes::check_for_class(L, index, lua_classes::euler::metatable_key))
         return _type::euler;
      if (classes::check_for_class(L, index, lua_classes::matrix3x3::metatable_key))
         return _type::matrix;
      if (classes::check_for_class(L, index, lua_classes::quaternion::metatable_key))
         return _type::quaternion;
      //
      lua_len(L, index);
      if (lua_isnumber(L, -1)) {
         int len = lua_tonumber(L, -1);
         lua_pop(L, 1);
         if (len == 9)
            return _type::matrix;
         if (len == 4)
            return _type::quaternion;
         //if (len == 3)
         //   return _type::euler; // NO! If the user passes { 1, 2, 3 } we have no way of knowing whether those are radians or degrees!
      }
      return _type::unknown;
   }

   static constexpr const std::array euler_axis_names      = { "x", "y", "z" };
   static constexpr const std::array quaternion_axis_names = { "w", "x", "y", "z" };
}

namespace dovahscript::api_helpers {
   extern bool pull_rotation(lua_State* L, int index, cobb::euler& result) {
      index = lua_absindex(L, index);
      if (!cobb::lua::istablelike(L, index))
         return false;
      //
      auto type = _get_table_type(L, index);
      if (type == _type::unknown)
         return false;
      switch (type) {
         case _type::euler:
            {
               auto& list = euler_axis_names;
               for (size_t i = 0; i < list.size(); ++i) {
                  lua_getfield(L, index, list[i]);
                  if (!lua_isnumber(L, -1)) {
                     lua_pop(L, 1);
                     return false;
                  }
               }
               result.x = lua_tonumber(L, -3);
               result.y = lua_tonumber(L, -2);
               result.z = lua_tonumber(L, -1);
               lua_pop(L, 3);
               return true;
            }
            break;
         case _type::quaternion:
            {
               auto& list = quaternion_axis_names;
               for (size_t i = 0; i < list.size(); ++i) {
                  lua_getfield(L, index, list[i]);
                  if (!lua_isnumber(L, -1)) {
                     lua_pop(L, 1);
                     lua_geti(L, 2, i + 1);
                     if (!lua_isnumber(L, -1)) {
                        lua_pop(L, 1);
                        return false;
                     }
                  }
               }
               cobb::quaternion value;
               value.w = lua_tonumber(L, -4);
               value.x = lua_tonumber(L, -3);
               value.y = lua_tonumber(L, -2);
               value.z = lua_tonumber(L, -1);
               lua_pop(L, 4);
               result = (cobb::euler)value;
               return true;
            }
            break;
         case _type::matrix:
            {
               cobb::rotation_matrix value;
               for (int i = 0; i < 9; ++i) {
                  lua_geti(L, index, i + 1);
                  if (!lua_isnumber(L, -1)) {
                     lua_pop(L, 1);
                     return false;
                  }
                  value.at(i) = lua_tonumber(L, -1);
                  lua_pop(L, 1);
               }
               result = (cobb::euler)value;
               return true;
            }
      }
      return false;
   }
}