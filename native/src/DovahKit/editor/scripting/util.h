#pragma once
#include "../../../Lua/lua.hpp"

namespace editor_script {
   using luastackchange_t = int;
   //
   namespace util {
      extern luastackchange_t safe_call(lua_State* luaVM, int arg_count, int return_count);
   }
}