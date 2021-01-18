#pragma once
#include "../../../Lua/lua.hpp"

namespace cobb::lua {
   class set_top_on_exit {
      protected:
         lua_State* state = nullptr;
         int pos;
      public:
         set_top_on_exit(lua_State* s, int p) : state(s), pos(p) {}
         ~set_top_on_exit() {
            lua_settop(this->state, this->pos);
         }
   };
}