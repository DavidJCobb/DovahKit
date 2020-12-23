#pragma once
#include "../../../../Lua/lua.hpp"
#include "../classes.h"

namespace editor_script::classes {
   class vector2 {
      public:
         static constexpr char* metatable_key = "dovah.classes.vector2";
         static luaL_Reg metatable_methods[];

         inline static bool check_arg_type(lua_State* L, int i) {
            return (editor_script::cast_to_class(L, i, metatable_key) != nullptr);
         }
         inline static void require_arg_type(lua_State* L, int i) {
            if (editor_script::cast_to_class(L, i, metatable_key) == nullptr)
               luaL_error(L, "bad argument #%i (expected %s)", i, metatable_key);
         }
         inline static void require_self_type(lua_State* L) {
            if (editor_script::cast_to_class(L, 1, metatable_key) == nullptr)
               luaL_error(L, "function called with bad self (expected %s)", metatable_key);
         }
   };
}