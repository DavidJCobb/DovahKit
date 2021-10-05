#include "safe_call.h"
#include <QString>
#include "core/subsystems/coordinator.h"
#include "core/specialized_traceback.h"
#include "dovahscript_host.h"

namespace {
   int _error_handler(lua_State* L) {
      auto& host = dovahscript::host::get();
      if (lua_type(L, -1) == LUA_TUSERDATA) {
         lua_getfield(L, LUA_REGISTRYINDEX, dovahscript::core::subsystems::coordinator::abort_sentinel_userdata);
         bool eq = lua_rawequal(L, -1, -2);
         lua_pop(L, 1);
         if (eq) {
            emit host.messageLogged("Script execution halted at the user's request.");
            return 1;
         }
      }
      auto original = lua_tostring(L, -1);
      //luaL_traceback(L, L, original, 1);
      dovahscript::core::specialized_traceback(L, original, 1);
      //
      auto message = QString::fromUtf8(lua_tostring(L, -1));
      emit host.messageLogged(message);
      //
      return 1;
   }
}

namespace dovahscript {
   extern int safe_call(lua_State* L, int arg_count, int return_count) {
      // STACK: - [ ..., function, args ] +
      int handler_pos = lua_gettop(L) - arg_count; // this is currently the position of the function to call, but we'll be moving the error handler here
      //
      // Add the error handler to the stack, and then move it before the Lua 
      // function and arguments:
      //
      lua_pushcfunction(L, _error_handler);
      // STACK: - [ ..., function, args, error handler ] +
      lua_insert(L, handler_pos);
      // STACK: - [ ..., error handler, function, args ] +
      auto ret = lua_pcall(L, arg_count, return_count, handler_pos);
      if (ret != LUA_OK)
         lua_pop(L, 1); // remove error object from stack
      // STACK: - [ ..., error handler, return values ] +
      lua_remove(L, handler_pos); // remove error handler from stack
      // STACK: - [ ..., return values ] +
      return ret;
   }
}