#include "util.h"
#include "editor_script_core.h"
#include "messages/log_text.h"

namespace {
   using namespace editor_script;
   //
   luastackchange_t _error_handler(lua_State* luaVM) {
      auto original = lua_tostring(luaVM, -1);
      luaL_traceback(luaVM, luaVM, original, 1);
      //
      auto* message = new messages::log_text();
      message->text = QString::fromUtf8(lua_tostring(luaVM, -1));
      DovahKitScriptVM::get()._send_message(message);
      //
      return 1;
   }
}

namespace editor_script::util {
   extern luastackchange_t safe_call(lua_State* luaVM, int arg_count, int return_count) {
      int handler_pos = lua_gettop(luaVM) - arg_count; // this is currently the position of the function to call, but we'll be moving the handler here
      //
      // Add the error handler to the stack, and then move it before the Lua 
      // function and arguments:
      //
      lua_pushcfunction(luaVM, _error_handler); // STACK: [error handler, args..., function]
      lua_insert(luaVM, handler_pos); // STACK: [args..., function, error handler]
      // STACK: [args..., function, error handler]
      //
      auto ret = lua_pcall(luaVM, arg_count, return_count, handler_pos);
      // STACK: [return values..., error handler]
      lua_remove(luaVM, -return_count - 1); // remove error handler from stack
      return ret;
   }
}