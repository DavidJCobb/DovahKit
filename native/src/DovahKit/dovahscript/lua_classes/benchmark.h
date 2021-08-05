#pragma once
#include <cstdint>
#include <windows.h>
#include "../../../helpers/intrusive_windows_defines.h"
#include "../../../lua.h"
#include "../core/classes.h"

namespace dovahscript::lua_classes {
   class benchmark {
      public:
         static constexpr const char* global_name   = "benchmark";
         static constexpr const char* metatable_key = "dovah.classes.benchmark";
         static std::initializer_list<luaL_Reg> metatable_methods;

         static void push_new_instance(lua_State* L);
         static void setup(lua_State*);

         benchmark();

         LARGE_INTEGER time;
         bool done = false;

         void finish();

         inline uint32_t microseconds() const noexcept {
            return this->time.QuadPart;
         }
         inline uint32_t milliseconds() const noexcept {
            return this->time.QuadPart / 1000;
         }
   };
}