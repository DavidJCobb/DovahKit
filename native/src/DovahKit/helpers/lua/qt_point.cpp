#include "qt_point.h"

namespace cobb::lua {
   extern QPoint  pull_qpoint(lua_State* L, int index, const char* error_prefix) {
      QPoint point;
      int    isnum;
      int    value;
      index = lua_absindex(L, index);
      //
      lua_getfield(L, index, "x");
      if (!lua_isnumber(L, -1)) {
         lua_pop(L, 1);
         lua_geti(L, index, 1);
      }
      value = lua_tointegerx(L, -1, &isnum);
      if (!isnum)
         luaL_error(L, "%s has no x-coordinate (both the first array element and the `x` key are either missing or aren't integers)", error_prefix);
      point.setX(value);
      lua_pop(L, 1);
      //
      lua_getfield(L, index, "y");
      if (!lua_isnumber(L, -1)) {
         lua_pop(L, 1);
         lua_geti(L, index, 2);
      }
      value = lua_tointegerx(L, -1, &isnum);
      if (!isnum)
         luaL_error(L, "%s has no y-coordinate (both the second array element and the `y` key are either missing or aren't integers)", error_prefix);
      point.setX(value);
      lua_pop(L, 1);
      //
      return point;
   }
   extern QPointF pull_qpointf(lua_State* L, int index, const char* error_prefix) {
      QPointF point;
      index = lua_absindex(L, index);
      //
      lua_getfield(L, index, "x");
      if (!lua_isnumber(L, -1)) {
         lua_pop(L, 1);
         lua_geti(L, index, 1);
         if (!lua_isnumber(L, -1))
            luaL_error(L, "%s has no x-coordinate (both the first array element and the `x` key are either missing or aren't numbers)", error_prefix);
      }
      point.setX(lua_tonumber(L, -1));
      lua_pop(L, 1);
      //
      lua_getfield(L, index, "y");
      if (!lua_isnumber(L, -1)) {
         lua_pop(L, 1);
         lua_geti(L, index, 2);
         if (!lua_isnumber(L, -1))
            luaL_error(L, "%s has no y-coordinate (both the second array element and the `y` key are either missing or aren't numbers)", error_prefix);
      }
      point.setY(lua_tonumber(L, -1));
      lua_pop(L, 1);
      //
      return point;
   }
}
