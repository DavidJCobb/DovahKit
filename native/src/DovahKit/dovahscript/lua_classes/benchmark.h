#pragma once
#include <cstdint>
#include "../../../lua.h"
#include "../core/classes.h"

namespace dovahscript::lua_classes {
   class benchmark {
      public:
         static constexpr const char* global_name   = "benchmark";
         static constexpr const char* metatable_key = "dovah.classes.benchmark";
         static std::initializer_list<luaL_Reg> metatable_methods;
         static std::initializer_list<luaL_Reg> metatable_getters;
         static std::initializer_list<luaL_Reg> metatable_setters;

         static void push_new_instance(lua_State* L);
         static void setup(lua_State*);
   };
}