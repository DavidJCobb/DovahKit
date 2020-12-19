#include "editor_script_core.h"
#include "util.h"

namespace {
   void _lua_debug_hook(lua_State* L, lua_Debug* ar) {
      auto& vm = DovahKitScriptVM::get();
      if (vm.is_aborted()) {
         luaL_error(L, "Script terminated at the user's request.");
         __assume(0); // luaL_error performs a jump and so does not return
      }
   }
   int _shimmed_pcall(lua_State* L) { // (pcall) shim to prevent userscripts from catching the error that (_lua_debug_hook) uses to force-kill a script
      int arg_count = lua_gettop(L) - 1;
      int status    = lua_pcall(L, arg_count, LUA_MULTRET, 0);
      if (status != LUA_OK) {
         //
         // Stack now contains only an error object.
         //
         if (DovahKitScriptVM::get().is_aborted()) {
            luaL_error(L, lua_tostring(L, -1));
            __assume(0); // luaL_error performs a jump and so does not return
         }
         //
         // The error is already on the stack, so let's just push the success bool 
         // and error text, and then we oughta be good.
         //
         lua_pushboolean(L, false); // stack after this: [error, false]
         lua_pushstring(L, lua_tostring(L, 1)); // stack: [error, false, "error"]
         lua_pop(L, 1); // remove the earliest-pushed element
         return 2;
      }
      int return_count = lua_gettop(L);
      lua_pushboolean(L, true);
      return return_count + 1;
   }
}

DovahKitScriptVM::DovahKitScriptVM() {
}
DovahKitScriptVM::~DovahKitScriptVM() {
   this->_teardown_lua_vm();
}

void DovahKitScriptVM::_setup_lua_vm() {
   this->lua_vm = luaL_newstate();
   lua_sethook(this->lua_vm, &_lua_debug_hook, LUA_MASKCOUNT, 8);
}
void DovahKitScriptVM::_teardown_lua_vm() {
   auto guard = std::lock_guard(this->exec_lock);
   if (this->lua_vm) {
      lua_close(this->lua_vm);
      this->lua_vm = nullptr;
   }
   this->running = false;
}

void DovahKitScriptVM::abort() {
   auto guard = std::lock_guard(this->exec_lock);
   if (this->running)
      this->aborted = true;
}
void DovahKitScriptVM::runScript(const QString& code, const QString& name) {
   auto guard = std::lock_guard(this->exec_lock);
   if (this->running)
      return;
   this->aborted = false;
   this->running = true;
   emit scriptStarted();
   this->_teardown_lua_vm();
   this->_setup_lua_vm();
   //
   auto buffer = code.toUtf8();
   auto result = luaL_loadbufferx(this->lua_vm, buffer.data(), buffer.size(), name.toUtf8().data(), "t"); // equivalent to (lua_load) with a built-in lua_Reader
   if (result == LUA_OK) {
      this->thread = std::thread([this]() {
         editor_script::util::safe_call(this->lua_vm, 0, 0);
         this->_teardown_lua_vm();
         emit this->scriptEnded(false);
      });
      return;
   }
   switch (result) {
      case LUA_ERRMEM:
         // TODO: log the error somehow
         break;
      case LUA_ERRSYNTAX:
         // TODO: log the error somehow
         break;
   }
   this->_teardown_lua_vm();
   emit scriptEnded(true);
}