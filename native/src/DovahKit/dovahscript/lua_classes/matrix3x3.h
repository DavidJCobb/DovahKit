#pragma once
#include "../../../lua.h"
#include "../../../helpers/rotation.h"
#include "../../../helpers/lua/error.h"
#include "../core/classes.h"

namespace dovahscript::lua_classes {
   class matrix3x3 {
      public:
         static constexpr const char* global_name   = "matrix3x3";
         static constexpr const char* metatable_key = "dovah.classes.matrix3x3";
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

         static cobb::rotation_matrix extract_from_stack(lua_State* L, int stack_pos);
         static void push_new_instance(lua_State* L, const cobb::rotation_matrix&);

         static void setup(lua_State*);
   };
}