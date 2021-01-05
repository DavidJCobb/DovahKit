#include "editor_script_core.h"
#include <array>
#include "util.h"
#include "api/allowed_standard_apis.h"
#include "cross_thread_tasks/_all.h"
#include "wrappers/_build_metatables.h"
#include "class_killer.h"

#include "../core.h" // for dovah.get_form_by_id
#include "api/form_type_values.h"
#include "wrapper_util.h"
#include "wrappers/form.h"
#include "classes/_all.h"
#include "../../helpers/lua/dump.h"

namespace {
   constexpr char* wrapper_storage_registry_key  = "dovah.internals.extant_wrappers";
   constexpr char* wrapper_weakmap_metatable_key = "__weakmap_mode_metatable";

   constexpr char* string_format_registry_key = "cached:string.format"; // key for a cached copy of (string.format), in case a script monkeypatches/replaces the original
}

namespace {
   void _lua_debug_hook(lua_State* L, lua_Debug* ar) {
      auto& vm = DovahKitScriptVM::get();
      if (vm.is_aborted()) {
         luaL_error(L, "Script terminated at the user's request.");
         __assume(0); // luaL_error performs a jump and so does not return
      }
   }

   void _lua_warning_function(void* ud, const char* msg, int tocont) {
      static QString text = "[Warning] "; // we could use (ud) to hold this, but since the VM is itself a singleton, no point in trying to allow multiple warning handlers to exist simultaneously
      text += msg;
      //
      if (!tocont) {
         auto* m = new editor_script::tasks::s2m::log_message();
         m->text = text;
         DovahKitScriptVMMessenger::get().send_message(m);
         //
         text = "[warning] ";
      }
   }

   int _shimmed_collectgarbage(lua_State* L) {
      luaL_argcheck(L, lua_isstring(L, 1), 1, "The argument must be a string.");
      if (strcmp(lua_tostring(L, 1), "collect") != 0) {
         luaL_error(L, "The only garbage-collection feature that this script environment allows access to is \"collect\".");
         __assume(0); // luaL_error performs a jump and so does not return
      }
      lua_gc(L, LUA_GCCOLLECT);
      return 0;
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
   int _shimmed_print(lua_State* L) {
      auto  m    = new editor_script::tasks::s2m::log_message();
      auto& text = m->text;
      //
      auto argcount = lua_gettop(L);
      for (int i = 1; i <= argcount; ++i) {
         size_t length;
         auto*  content = luaL_tolstring(L, i, &length);
         if (i > 1)
            text += '\t';
         text += QString::fromUtf8(content, length);
         lua_pop(L, 1);
      }
      //
      DovahKitScriptVMMessenger::get().send_message(m);
      return 0;
   }
   int _wrapper_is_zombie(lua_State* L) {
      lua_settop(L, 1);
      lua_pushboolean(L, editor_script::userdata_is_zombie(L, 1));
      return 1;
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
         luastackchange_t benchmark_start(lua_State* L) {
            auto* p = lua_newuserdata(L, sizeof(classes::benchmark)); // push 1
            luaL_getmetatable(L, classes::benchmark::metatable_key); // push 1
            lua_setmetatable(L, -2); // pop 1
            new (p) classes::benchmark;
            return 1;
         }
         luastackchange_t benchmark_stop(lua_State* L) {
            auto* self = (classes::benchmark*) editor_script::cast_to_exact_class(L, 1, classes::benchmark::metatable_key);
            if (self == nullptr) {
               luaL_error(L, "bad argument #1 to dovah.benchmark_stop (expected %s)", classes::benchmark::metatable_key);
            }
            __assume(self != nullptr);
            self->finish();
            return 0;
         }
         luastackchange_t for_each_form_of_type(lua_State* L) {
            luaL_argcheck(L, lua_isnumber(L, 1),   1, "form type (number) expected");
            luaL_argcheck(L, lua_isfunction(L, 2), 2, "function expected");
            auto& editor = DovahKitCore::get();
            if (!editor.has_data())
               return 0;
            //
            bool  valid = false;
            auto  ft    = editor_script::get_form_type_from_stack(L, 1, valid);
            if (!valid)
               return 0;
            auto& info  = ::dovah::form_type_info::lookup(ft);
            if (info.flags & ::dovah::form_type_info::flag::is_singleton) {
               //
               // For singleton forms, only use the canonical stub.
               //
               auto* stub = editor.get_singleton_form(ft, false);
               if (stub) {
                  lua_pushvalue(L, 2); // push the function
                  wrapper out;
                  auto*   mt = wrap_form(out, stub);
                  if (DovahKitScriptVMUserdataInterface::get().push(L, out, mt))
                     lua_call(L, 1, 1);
               }
               return 0;
            }
            //
            editor.for_each_form_of_type(ft, [L](::dovah::form_stub* stub) {
               lua_pushvalue(L, 2); // push the function
               wrapper out;
               auto*   mt = wrap_form(out, stub);
               if (DovahKitScriptVMUserdataInterface::get().push(L, out, mt)) {
                  lua_call(L, 1, 1);
                  if (lua_toboolean(L, -1) == 1) {
                     return true;
                  }
               } else {
                  lua_settop(L, 2);
               }
               return false;
            });
            //
            return 0;
         }
         luastackchange_t get_form_by_id(lua_State* L) {
            luaL_argcheck(L, lua_isnumber(L, 1), 1, "form ID (number) expected");
            auto& editor = DovahKitCore::get();
            if (!editor.has_data())
               return 0;
            auto  id   = lua_tonumber(L, 1);
            auto* stub = editor.get_form(id);
            if (!stub)
               return 0;
            //
            wrapper out;
            auto*   mt = wrap_form(out, stub);
            return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
         }
         luastackchange_t log_message(lua_State* L) {
            auto m = new editor_script::tasks::s2m::log_message();
            //
            auto argcount = lua_gettop(L);
            if (!argcount)
               return 0;
            //
            if (lua_type(L, 1) != LUA_TSTRING) { // coerce argument 1 to a string if it isn't one, as string.format doesn't do this automatically
               luaL_tolstring(L, 1, nullptr);
               lua_copy(L, argcount + 1, 1);
               lua_pop(L, 1);
            }
            //
            lua_getfield(L, LUA_REGISTRYINDEX, string_format_registry_key);
            if (lua_isfunction(L, argcount + 1)) {
               lua_rotate(L, 1, 1); // move (string.format) ahead of the other stack elements
               lua_call  (L, argcount, 1);
            }
            //
            const char* out = lua_tostring(L, 1);
            if (!out) {
               out = "";
            }
            m->text = QString::fromUtf8(out);
            //
            DovahKitScriptVMMessenger::get().send_message(m);
            return 0;
         }
         luastackchange_t test_call_and_response(lua_State* L) {
            auto* m = new editor_script::tasks::s2m::test_call_and_response();
            DovahKitScriptVMMessenger::get().send_message(m);
            return 0;
         }
      }
   }
   namespace declarations {
      std::array dovah = {
         function{ "benchmark_start",        &definitions::dovah::benchmark_start },
         function{ "benchmark_stop",         &definitions::dovah::benchmark_stop },
         function{ "for_each_form_of_type",  &definitions::dovah::for_each_form_of_type },
         function{ "get_form_by_id",         &definitions::dovah::get_form_by_id },
         function{ "log_message",            &definitions::dovah::log_message },
         function{ "test_call_and_response", &definitions::dovah::test_call_and_response },
      };
   }
}

void DovahKitScriptVM::_task_queue::process() {
   auto  guard = std::lock_guard(this->lock);
   auto& list  = this->list;
   //
   for (auto* task : list) {
      bool blocking = task->is_blocking();
      task->execute();
      if (!blocking && task->is_fire_and_forget())
         delete task;
   }
   list.clear();
}

#pragma region DovahKitScriptVM
DovahKitScriptVM::DovahKitScriptVM() {
   this->main_thread_tick_timer.setSingleShot(false);
   this->main_thread_tick_timer.setInterval(0);
   QObject::connect(this, &DovahKitScriptVM::scriptStarted, this, [this]() { this->main_thread_tick_timer.start(); });
   QObject::connect(this, &DovahKitScriptVM::scriptEnded,   this, [this]() { this->main_thread_tick_timer.stop(); });
   //
   QObject::connect(&this->main_thread_tick_timer, &QTimer::timeout, this, &DovahKitScriptVM::mainThreadLoop);
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool will_be_flagged) {
      auto* message = new editor_script::tasks::m2s::form_deleted;
      message->stub = stub;
      //
      auto  guard   = std::lock_guard(this->task_queues.m2s.urgent.lock);
      auto& list    = this->task_queues.m2s.urgent.list;
      list.push_back(message);
   });
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
   lua_setwarnf(this->lua_vm, &_lua_warning_function, nullptr);
   //
   // Make the appropriate standard libraries available, and prune any functions that we 
   // don't want the user having easy access to:
   //
   luaL_requiref(this->lua_vm, "_G",     luaopen_base,  1); // loads the library to the top of the Lua stack
   {  // shim collectgarbage
      auto ti = lua_gettop(this->lua_vm);
      lua_pushstring   (this->lua_vm, "collectgarbage");
      lua_pushcfunction(this->lua_vm, &_shimmed_collectgarbage);
      lua_rawset       (this->lua_vm, ti);
   }
   {  // shim pcall
      auto ti = lua_gettop(this->lua_vm);
      lua_pushstring   (this->lua_vm, "pcall");
      lua_pushcfunction(this->lua_vm, &_shimmed_pcall);
      lua_rawset       (this->lua_vm, ti);
   }
   {  // shim print
      auto ti = lua_gettop(this->lua_vm);
      lua_pushstring   (this->lua_vm, "print");
      lua_pushcfunction(this->lua_vm, &_shimmed_print);
      lua_rawset       (this->lua_vm, ti);
   }
   {  // object_is_zombie
      auto ti = lua_gettop(this->lua_vm);
      lua_pushstring   (this->lua_vm, "object_is_zombie");
      lua_pushcfunction(this->lua_vm, &_wrapper_is_zombie);
      lua_rawset       (this->lua_vm, ti);
   }
   editor_script::prune_standard_library(this->lua_vm, "basic"); // also pops the library from the Lua stack
   luaL_requiref(this->lua_vm, "debug",  luaopen_debug, 1);
   editor_script::prune_standard_library(this->lua_vm, "debug");
   luaL_requiref(this->lua_vm, "math",   luaopen_math, 1);
   editor_script::prune_standard_library(this->lua_vm, "math");
   luaL_requiref(this->lua_vm, "string", luaopen_string, 1);
   editor_script::prune_standard_library(this->lua_vm, "string");
   luaL_requiref(this->lua_vm, "table",  luaopen_table, 1);
   editor_script::prune_standard_library(this->lua_vm, "table");
   luaL_requiref(this->lua_vm, "utf8",   luaopen_utf8, 1);
   editor_script::prune_standard_library(this->lua_vm, "utf8");
   //
   lua_getglobal(this->lua_vm, "string");
   lua_getfield (this->lua_vm, -1, "format");
   lua_setfield (this->lua_vm, LUA_REGISTRYINDEX, string_format_registry_key);
   lua_pop(this->lua_vm, 1);
   //
   editor_script::expose_form_types_to_lua(this->lua_vm);
   //
   // Prepare API classes:
   //
   #pragma region Wrapper storage table
      lua_newtable(this->lua_vm);
      lua_setfield(this->lua_vm, LUA_REGISTRYINDEX, wrapper_storage_registry_key);
      //
      lua_newtable  (this->lua_vm);
      lua_pushstring(this->lua_vm, "v");
      lua_setfield  (this->lua_vm, -2, "__mode");
      lua_setfield(this->lua_vm, LUA_REGISTRYINDEX, wrapper_weakmap_metatable_key);
   #pragma endregion
   editor_script::build_all_wrapper_metatables(this->lua_vm);
   editor_script::define_class(this->lua_vm, editor_script::classes::benchmark::metatable_key, nullptr, editor_script::classes::benchmark::metatable_methods);
   editor_script::classes::euler::setup(this->lua_vm);
   editor_script::classes::matrix3x3::setup(this->lua_vm);
   editor_script::classes::quaternion::setup(this->lua_vm);
   editor_script::classes::vector2::setup(this->lua_vm);
   editor_script::classes::vector3::setup(this->lua_vm);
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

void DovahKitScriptVM::_script_thread_loop() {
   editor_script::util::safe_call(this->lua_vm, 0, 0);
   //
   while (this->_should_keep_running()) {
      this->task_queues.m2s.urgent.process();
      this->task_queues.m2s.normal.process();
   }
   //
   this->_teardown_lua_vm();
   this->running = false;
   emit this->scriptEnded(false);
}

bool DovahKitScriptVM::_should_keep_running() const noexcept {
   //
   // TODO: If the script has any script-spawned UI windows open and visible, then 
   // this should return (true). If we want to be more sophisticated, then we can 
   // double-check that the windows or any controls in them have any event listeners 
   // registered.
   //
   // The basic thing we're checking for is, "We're not running script code *right 
   // now*, but can we *end up* running them as a result of any extant event 
   // listeners?"
   //
   return false;
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
      this->thread = std::thread(&DovahKitScriptVM::_script_thread_loop, this);
      return;
   }
   switch (result) {
      case LUA_ERRMEM:
      case LUA_ERRSYNTAX:
      default:
         auto message = QString::fromUtf8(lua_tostring(this->lua_vm, -1));
         emit DovahKitScriptVM::get().messageLogged(message);
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
   this->task_queues.s2m.process();
}
#pragma endregion

#pragma region DovahKitScriptVMMessenger
void DovahKitScriptVMMessenger::send_message(editor_script::cross_thread_task* m) {
   auto& vm = DovahKitScriptVM::get();
   {
      auto  guard = std::lock_guard(vm.task_queues.s2m.lock);
      auto& list  = vm.task_queues.s2m.list;
      list.push_back(m);
   }
   if (m->is_blocking()) {
      while (!m->seen)
         if (vm.is_aborted())
            break;
      vm.task_queues.m2s.urgent.process();
   }
}
#pragma endregion 

#pragma region
/*static*/ void DovahKitScriptVMPermissionInterface::verify_form_write_permissions() {
   auto& intfc = DovahKitScriptVMPermissionInterface::get();
   if (false) { // TODO: permission check, when we implement those
      luaL_error(intfc.vm.lua_vm, "The script does not have permission to use APIs that modify form data.");
      __assume(0);
   }
}
#pragma endregion

void DovahKitScriptVMUserdataInterface::remove(editor_script::wrapper& instance) {
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   auto  index = instance.last_part().index;
   void* light = instance.get_pertinent_pointer();
   //
   std::vector<int> refs_to_sever;
   refs_to_sever.push_back(instance.lua_key);
   //
   auto si_storage = start + 1;
   auto si_nk      = start + 2;
   auto si_nv      = start + 3;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key); // push 1
   lua_pushlightuserdata(L, light);
   lua_rawget(L, si_storage); // STACK: - [ ..., storage_root, storage_root[light] ] +
   assert(lua_istable(L, -1));
   lua_copy  (L, -1, si_storage);
   lua_settop(L, si_storage); // STACK: - [ ..., storage_root[light] ] +
   //
   // If the wrapper to be removed is in a sequential collection, fix up the indices of all 
   // of its next-siblings. Either way, identify and track the keys of any child/descendant 
   // wrappers.
   //
   lua_pushnil(L); // nk
   while (lua_next(L, si_storage) != 0) {
      if (lua_type(L, si_nk) == LUA_TNUMBER && lua_tonumber(L, si_nk) == 0.0) {
         //
         // luaL_ref and friends use key 0 to store a list of free indices. we need to 
         // manually ignore it.
         //
         lua_settop(L, si_nk);
         continue;
      }
      //
      editor_script::wrapper* other = nullptr;
      if (lua_type(L, si_nv) == LUA_TUSERDATA)
         other = (editor_script::wrapper*) lua_touserdata(L, si_nv);
      //
      lua_settop(L, si_nk);
      //
      if (!other || other->lua_key == instance.lua_key)
         continue;
      if (instance.is_in_same_collection(*other)) {
         auto& o_last = other->last_part();
         if (index < o_last.index)
            //
            // reduce (other), as a previous sibling has been deleted.
            //
            --o_last.index;
      } else if (other->is_descendant_of(instance)) {
         refs_to_sever.push_back(other->lua_key);
      }
   }
   //
   // Zombify and forget the wrapper and all of its descendants.
   //
   lua_settop(L, si_storage);
   for (auto key : refs_to_sever) {
      lua_pushcfunction(L, &editor_script::zombify_userdata); // prepare to make a Lua call...
      lua_rawgeti      (L, si_storage, key);
      //
      auto* target = (editor_script::wrapper*) lua_touserdata(L, -1);
      assert(target && target->lua_key == key);
      target->lua_key = LUA_NOREF;
      target->stub    = nullptr; // need to sever this now, because we won't be able to if, say, the form is deleted after we forget about this wrapper
      target->form    = nullptr;
      //
      lua_call(L, 1, 0); // ...and then, after we've adjusted the native wrapper, make the call.
      //
      luaL_unref(L, si_storage, key); // remove the target from storage.
   }
   //
   lua_pushnil(L);
   if (lua_next(L, si_storage) == 0) { // table is empty
      //
      // If the list of wrappers for this pointer is empty, delete the list itself.
      //
      lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key);
      lua_pushlightuserdata(L, light);
      lua_pushnil(L);
      lua_rawset(L, -3);
   }
   //
   lua_settop(L, start);
}

void DovahKitScriptVMUserdataInterface::remove_form(dovah::form_stub& stub) {
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   //
   auto si_storage = start + 1;
   auto si_nk      = start + 2;
   auto si_nv      = start + 3;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key); // push 1
   lua_pushlightuserdata(L, &stub);
   lua_rawget(L, si_storage); // STACK: - [ ..., storage_root, storage_root[light] ] +
   if (!lua_istable(L, -1)) {
      lua_settop(L, start);
      return;
   }
   lua_copy  (L, -1, si_storage);
   lua_settop(L, si_storage); // STACK: - [ ..., storage_root[light] ] +
   //
   // Zombify all wrappers for this form and its parts.
   //
   lua_pushnil(L); // nk
   while (lua_next(L, si_storage) != 0) {
      if (lua_type(L, si_nk) == LUA_TNUMBER && lua_tonumber(L, si_nk) == 0.0) {
         //
         // luaL_ref and friends use key 0 to store a list of free indices. we need to 
         // manually ignore it.
         //
         lua_settop(L, si_nk);
         continue;
      }
      //
      editor_script::wrapper* other = nullptr;
      if (lua_type(L, si_nv) == LUA_TUSERDATA) {
         if (auto* target = (editor_script::wrapper*) lua_touserdata(L, si_nv)) {
            assert(target->stub == &stub);
            target->lua_key = LUA_NOREF;
            target->stub    = nullptr; // need to sever this now, because we won't be able to if, say, the form is deleted after we forget about this wrapper
            target->form    = nullptr;
            //
            lua_pushcfunction(L, &editor_script::zombify_userdata);
            lua_pushvalue    (L, si_nv);
            lua_call(L, 1, 0);
         }
      }
      //
      lua_settop(L, si_nk);
   }
   //
   // Erase the table for this form.
   //
   lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key);
   lua_pushlightuserdata(L, &stub);
   lua_pushnil(L);
   lua_rawset(L, -3);
   //
   lua_settop(L, start);
}

int DovahKitScriptVMUserdataInterface::push(lua_State* L, const editor_script::wrapper& instance, const char* metatable_name) {
   auto  start = lua_gettop(L);
   lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key); // push 1
   auto  table = lua_gettop(L);
   //
   auto si_storage = start + 1;
   auto si_nk      = start + 2;
   auto si_nv      = start + 3;
   auto si_created = si_storage + 1;
   //
   void* light = instance.get_pertinent_pointer();
   {  // Get the list of wrappers for this pointer
      lua_pushlightuserdata(L, light); // push 1
      lua_rawget(L, table);            // push 0 // STACK: - [ ..., storage_root, storage_root[light] ] +
      if (!lua_istable(L, -1)) {
         lua_settop(L, table);
         //
         lua_createtable (L, 0, 0); // push 1 // storage_sub = {}
         lua_getfield    (L, LUA_REGISTRYINDEX, wrapper_weakmap_metatable_key);
         lua_setmetatable(L, -2);
         lua_pushlightuserdata(L, light);     // push 1
         lua_pushvalue        (L, table + 1); // push 1
         lua_rawset           (L, table);     // pop  2 // STACK: - [ ..., storage_root, storage_root[light] ] + // storage_root[wrapper_pointer] = storage_sub
      }
      lua_copy  (L, -1, table);
      lua_settop(L, table); // STACK: - [ ..., storage_root[light] ] + // table = registry[wrapper_storage_registry_key][wrapper_pointer] or {}
   }
   //
   // Check for an existing identical wrapper:
   //
   lua_pushnil(L); // push 1
   while (lua_next(L, table) != 0) { // push 2 (only if truthy)
      if (lua_type(L, si_nk) == LUA_TNUMBER && lua_tonumber(L, si_nk) == 0.0) {
         //
         // luaL_ref and friends use key 0 to store a list of free indices. we need to 
         // manually ignore it.
         //
         lua_settop(L, si_nk);
         continue;
      }
      auto* existing = (editor_script::wrapper*) lua_touserdata(L, si_nv);
      #if _DEBUG
         if (!existing) {
            cobb::lua::print_stack_and_vars(L);
            __debugbreak();
         }
      #endif
      if (existing->is_equal(&instance)) {
         lua_copy  (L, si_nv, table); // move the value
         lua_settop(L, table);
         return 1;
      }
      lua_pop(L, 1); // pop the value; keep the key for the next iteration
   }
   //
   assert(lua_gettop(L) == si_storage);
   //
   // Create a new wrapper.
   //
   auto* ptr = (editor_script::wrapper*) lua_newuserdatauv(L, sizeof(editor_script::wrapper), 0); // push 1
   new (ptr) editor_script::wrapper;
   *ptr = instance;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, metatable_name); // push 1
   if (lua_isnoneornil(L, -1)) {
      assert(false && "The wrapper-class wasn't set up properly; its metatable is undefined.");
      lua_settop(L, start);
      return 0;
   }
   lua_setmetatable(L, si_created); // pop 1
   //
   lua_pushvalue(L, si_created); // push 1 // push another reference to the wrapper onto the stack, as the next function will remove whichever reference it uses
   ptr->lua_key = luaL_ref(L, si_storage);
   //
   lua_remove(L, -2);
   return 1;
}

void DovahKitScriptVMUserdataInterface::remove_from_sequential_collection(editor_script::wrapper& to_remove) {
   assert(to_remove.depth && !to_remove.is_collection && "The (to_remove) argument must be an element in a sequential collection.");
   this->remove(to_remove);
}