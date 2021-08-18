#pragma once
#include "../../../lua.h"
#include "../../../helpers/rotation.h"
#include "../../../helpers/lua/error.h"
#include "../core/classes.h"

namespace dovahscript::lua_classes {
   class quaternion {
      public:
         static constexpr const char* global_name   = "quaternion";
         static constexpr const char* metatable_key = "dovah.classes.quaternion";
         static std::initializer_list<luaL_Reg> metatable_methods;

         inline static bool check_arg_type(lua_State* L, int i) {
            return classes::check_for_class(L, i, metatable_key);
         }
         inline static void require_arg_type(lua_State* L, int i) {
            if (!classes::check_for_class(L, i, metatable_key))
               cobb::lua::error(L, "bad argument #%i (expected %s)", i, metatable_key);
         }
         inline static void require_self_type(lua_State* L) {
            if (!classes::check_for_class(L, 1, metatable_key))
               cobb::lua::error(L, "function called with bad self (expected %s)", metatable_key);
         }

         static cobb::quaternion extract_from_stack(lua_State* L, int stack_pos);
         static void push_new_instance(lua_State* L, const cobb::quaternion&);

         static void setup(lua_State*);
   };
}