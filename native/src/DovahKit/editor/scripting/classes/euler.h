#pragma once
#include "../../../../Lua/lua.hpp"
#include "../../../helpers/rotation.h"
#include "../classes.h"

namespace editor_script::classes {
   class euler {
      public:
         static constexpr const char* global_name   = "euler";
         static constexpr const char* metatable_key = "dovah.classes.euler";
         static std::initializer_list<luaL_Reg> metatable_methods;

         inline static bool check_arg_type(lua_State* L, int i) {
            return editor_script::check_for_class(L, i, metatable_key);
         }
         inline static void require_arg_type(lua_State* L, int i) {
            if (!editor_script::check_for_class(L, i, metatable_key))
               luaL_error(L, "bad argument #%i (expected %s)", i, metatable_key);
         }
         inline static void require_self_type(lua_State* L) {
            if (!editor_script::check_for_class(L, 1, metatable_key))
               luaL_error(L, "function called with bad self (expected %s)", metatable_key);
         }

         static void push_new_instance(lua_State* L, const cobb::euler&);

         static void setup(lua_State*);
   };
}