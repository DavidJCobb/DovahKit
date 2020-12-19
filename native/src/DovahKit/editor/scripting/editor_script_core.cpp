#include "editor_script_core.h"
#include <array>
#include "util.h"
#include "messages/all.h"

#include <QMessageBox> // for test_call_and_response

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

namespace _api { // APIs
   using namespace editor_script;
   struct function {
      using ptr_t = luastackchange_t(*)(lua_State*);

      const char* name;
      ptr_t pointer = nullptr;
   };

   namespace definitions {
      namespace dovah {
         luastackchange_t log_message(lua_State* L) {
            auto m = new editor_script::messages::log_text();
            //
            const char* out = nullptr;
            if (lua_isstring(L, 1)) {
               out = lua_tostring(L, 1);
            } else {
               out = ""; // TODO: stringify value if possible
            }
            m->text = QString::fromUtf8(out);
            //
            DovahKitScriptVMMessenger::get().send_message(m);
            return 0;
         }
         luastackchange_t test_call_and_response(lua_State* L) {
            auto* m = new editor_script::messages::test_call_and_response();
            DovahKitScriptVMMessenger::get().send_message(m);
            return 0;
         }
      }
   }
   namespace declarations {
      std::array dovah = {
         function{ "log_message",            &definitions::dovah::log_message },
         function{ "test_call_and_response", &definitions::dovah::test_call_and_response },
      };
   }
}

#pragma region DovahKitScriptVM
DovahKitScriptVM::DovahKitScriptVM() {
   this->main_thread_tick_timer.setSingleShot(false);
   this->main_thread_tick_timer.setInterval(0);
   QObject::connect(this, &DovahKitScriptVM::scriptStarted, this, [this]() { this->main_thread_tick_timer.start(); });
   QObject::connect(this, &DovahKitScriptVM::scriptEnded,   this, [this]() { this->main_thread_tick_timer.stop(); });
   //
   QObject::connect(&this->main_thread_tick_timer, &QTimer::timeout, this, &DovahKitScriptVM::mainThreadLoop);
}
DovahKitScriptVM::~DovahKitScriptVM() {
   this->abort();
   if (this->thread.joinable()) // even if it's finished running, we need to join it or std::thread::operator= below will break
      this->thread.join();
   this->_teardown_lua_vm();
   this->running = false;
}

void DovahKitScriptVM::_setup_lua_vm() {
   this->lua_vm = luaL_newstate();
   lua_sethook(this->lua_vm, &_lua_debug_hook, LUA_MASKCOUNT, 8);
   //
   // Make API functions available via tables:
   //
   lua_newtable(this->lua_vm); // create a new table
   for (auto& entry : _api::declarations::dovah) {
      lua_pushstring   (this->lua_vm, entry.name);    // key
      lua_pushcfunction(this->lua_vm, entry.pointer); // value
      lua_rawset(this->lua_vm, -3);
   }
   lua_setglobal(this->lua_vm, "dovah"); // assign the new table to a variable
}
void DovahKitScriptVM::_teardown_lua_vm() {
   auto guard = std::lock_guard(this->exec_lock);
   if (this->lua_vm) {
      lua_close(this->lua_vm);
      this->lua_vm = nullptr;
   }
}

void DovahKitScriptVM::_send_outbound_message(editor_script::message* message) {
   auto  guard = std::lock_guard(this->outbound_message_queue.lock);
   auto& list  = this->outbound_message_queue.list;
   list.push_back(message);
}

void DovahKitScriptVM::view_messages(std::function<bool(editor_script::message*)> functor) {
   auto  guard = std::lock_guard(this->outbound_message_queue.lock);
   auto& list  = this->outbound_message_queue.list;
   //
   std::vector<editor_script::message*> remaining;
   for (auto* message : list) {
      bool blocking = message->is_blocking();
      bool resolved = (functor)(message);
      if (resolved) {
         message->seen = true;
         if (!blocking)
            delete message;
      } else {
         remaining.push_back(message);
      }
   }
   std::swap(list, remaining);
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
   if (this->thread.joinable()) // even if it's finished running, we need to join it or std::thread::operator= below will break
      this->thread.join();
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
         this->running = false;
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
   this->running = false;
   emit scriptEnded(true);
}

void DovahKitScriptVM::setUIParentWidget(QWidget* widget) {
   auto guard = std::lock_guard(this->exec_lock);
   if (this->running)
      return;
   this->ui_parent = widget;
}

void DovahKitScriptVM::mainThreadLoop() {
   this->view_messages([this](editor_script::message* message) {
      using namespace editor_script;
      //
      switch (message->type) {
         case message_type::log_text:
            if (auto* casted = dynamic_cast<messages::log_text*>(message)) {
               emit this->messageLogged(casted->text);
            }
            return true;
         case message_type::test_call_and_response:
            if (auto* casted = dynamic_cast<messages::test_call_and_response*>(message)) {
               QMessageBox::information(this->ui_parent, "Test", "This should block script execution until it is dismissed");
            }
            return true;
      }
      #if _DEBUG
         __debugbreak(); // Unhandled message type!
      #endif
      return message->is_blocking(); // don't let unrecognized blocking messages hang the script
   });
}
#pragma endregion

#pragma region DovahKitScriptVMMessenger
void DovahKitScriptVMMessenger::send_message(editor_script::message* m) {
   auto& vm = DovahKitScriptVM::get();
   vm._send_outbound_message(m);
   if (m->is_blocking())
      while (!m->seen)
         if (vm.is_aborted())
            break;
}
#pragma endregion 