#pragma once
#include <cstdint>
#include <windows.h>
#include "../../../helpers/intrusive_windows_defines.h"
#include "../../../../Lua/lua.hpp"
#include "../classes.h"

namespace editor_script::classes {
   class benchmark final {
      public:
         static constexpr char* metatable_key = "dovah.classes.benchmark";
         static luaL_Reg metatable_methods[];

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