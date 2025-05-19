#pragma once
#include "../../lua.h"
#include "helpers/lua/error.h"
#include "../core/classes.h"

namespace dovahscript::lua_classes {
   class vector3 {
      public:
         static constexpr const char* global_name   = "vector3";
         static constexpr const char* metatable_key = "dovah.classes.vector3";
         static std::initializer_list<luaL_Reg> metatable_methods;

         inline static bool check_arg_type(lua_State* L, int i) {
            return dovahscript::classes::check_for_class(L, i, metatable_key);
         }
         inline static void require_arg_type(lua_State* L, int i) {
            if (!dovahscript::classes::check_for_class(L, i, metatable_key))
               cobb::lua::error(L, "bad argument #%i (expected %s)", i, metatable_key);
         }
         inline static void require_self_type(lua_State* L) {
            if (!dovahscript::classes::check_for_class(L, 1, metatable_key))
               cobb::lua::error(L, "function called with bad self (expected %s)", metatable_key);
         }

         static void push_new_instance(lua_State* L, lua_Number x, lua_Number y, lua_Number z);

         static void setup(lua_State*);
   };
}