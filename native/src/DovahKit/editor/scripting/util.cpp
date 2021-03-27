#include "util.h"
#include "editor_script_core.h"

#include "../../helpers/lua/dump.h"

namespace {
   using namespace editor_script;
   //
   luastackchange_t _error_handler(lua_State* luaVM) {
      auto original = lua_tostring(luaVM, -1);
      luaL_traceback(luaVM, luaVM, original, 1);
      //
      auto message = QString::fromUtf8(lua_tostring(luaVM, -1));
      emit DovahKitScriptVM::get().messageLogged(message);
      //
      return 1;
   }
}

namespace editor_script::util {
   extern luastackchange_t safe_call(lua_State* luaVM, int arg_count, int return_count) {
      // STACK: - [ ..., function, args ] +
      int handler_pos = lua_gettop(luaVM) - arg_count; // this is currently the position of the function to call, but we'll be moving the error handler here
      //
      // Add the error handler to the stack, and then move it before the Lua 
      // function and arguments:
      //
      lua_pushcfunction(luaVM, _error_handler);
      // STACK: - [ ..., function, args, error handler ] +
      lua_insert(luaVM, handler_pos);
      // STACK: - [ ..., error handler, function, args ] +
      auto ret = lua_pcall(luaVM, arg_count, return_count, handler_pos);
      if (ret != LUA_OK)
         lua_pop(luaVM, 1); // remove error object from stack
      // STACK: - [ ..., error handler, return values ] +
      lua_remove(luaVM, handler_pos); // remove error handler from stack
      // STACK: - [ ..., return values ] +
      return ret;
   }
}