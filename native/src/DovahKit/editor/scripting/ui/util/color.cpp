#include "color.h"
#include <array>
#include "../../../../lua.h"

struct lua_State;

namespace editor_script::util::ui {
   extern void push_color(lua_State* L, const QColor& color) {
      lua_createtable(L, 4, 4);
      auto pos = lua_gettop(L);
      //
      std::array names = { "r", "g", "b", "a" };
      std::array parts = { color.red(), color.green(), color.blue(), color.alpha() };
      for (int i = 0; i < names.size(); ++i) {
         lua_pushinteger(L, parts[i]);
         lua_seti(L, pos, i + 1);
         lua_pushinteger(L, parts[i]);
         lua_setfield(L, pos, names[i]);
      }
   }
   extern [[nodiscard]] QColor pull_color(lua_State* L, int index) {
      QColor color;
      //
      index = lua_absindex(L, index);
      auto top = lua_gettop(L);
      switch (lua_type(L, index)) {
         case LUA_TTABLE:
            [[fallthrough]];
         case LUA_TUSERDATA:
            {
               int isnum;
               int value;
               std::array<int, 4> values = { 0, 0, 0, 255 };
               //
               constexpr std::array names = { "r", "g", "b" };
               for (int i = 1; i <= names.size(); ++i) {
                  lua_getfield(L, index, names[i - 1]);
                  isnum;
                  value = lua_tointegerx(L, top + 1, &isnum);
                  lua_pop(L, 1);
                  if (!isnum) {
                     lua_geti(L, 2, i);
                     value = lua_tointegerx(L, top + 1, &isnum);
                     lua_pop(L, 1);
                  }
                  values[i - 1] = value;
               }
               color.setRgb(values[0], values[1], values[2]);
               //
               lua_getfield(L, index, "a");
               value = lua_tointegerx(L, top + 1, &isnum);
               lua_pop(L, 1);
               if (isnum) {
                  values[3] = value;
               } else {
                  lua_geti(L, index, 4);
                  value = lua_tointegerx(L, top + 1, &isnum);
                  lua_pop(L, 1);
                  if (isnum)
                     values[3] = value;
               }
               color.setAlpha(values[3]);
            }
            break;
            //
         case LUA_TSTRING:
            [[fallthrough]];
         case LUA_TNUMBER:
         case LUA_TFUNCTION:
         case LUA_TLIGHTUSERDATA:
         case LUA_TBOOLEAN:
            [[fallthrough]];
         case LUA_TNONE:
         case LUA_TNIL:
            luaL_argerror(L, index, "table or nil expected");
            break;
      }
      //
      return color;
   }
}