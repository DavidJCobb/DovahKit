#pragma once
#include <cstdint>
#include <windows.h>
#include "../../../helpers/intrusive_windows_defines.h"
#include "../../../../Lua/lua.hpp"
#include "../classes.h"

namespace editor_script::classes {
   class vector3 {
      public:
         static constexpr char* metatable_key = "dovah.classes.vector3";
         static luaL_Reg metatable_methods[];
   };
}