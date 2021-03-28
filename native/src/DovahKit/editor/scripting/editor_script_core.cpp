#include "editor_script_core.h"
#include <array>
#include "util.h"
#include "api/allowed_standard_apis.h"
#include "cross_thread_tasks/_all.h"
#include "wrappers/_build_metatables.h"
#include "wrappers/_build_singletons.h"
#include "class_killer.h"

#include "../core.h" // needed for DovahKitCore signals
#include "../../dovah/forms/Form.h" // needed for any loaded_form_ptr
#include "api/form_type_values.h"
#include "classes/_all.h"
#include "../../helpers/lua/dump.h"
#include "../../helpers/lua/isempty.h"
#include "../../helpers/lua/qt_variant.h"
#include "../../helpers/lua/set_top_on_exit.h"

#include <QPushButton>

#include "api/namespaces/dovah.h"

namespace {
   constexpr char* wrapper_storage_registry_key  = "dovah.internals.extant_wrappers";
   constexpr char* wrapper_weakmap_metatable_key = "__weakmap_mode_metatable";

   constexpr char* ui_listener_registry_key = "dovah.internals.ui_listeners"; // registry[key][widget_pointer][event_name][listener_name]

   constexpr int max_script_windows = 10;
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
void DovahKitScriptVM::_task_queue::wait_until_empty() {
   auto& list = this->list;
   while (!list.empty()) {
   }
   auto& vm = DovahKitScriptVM::get();
   if (!vm.is_aborted()) {
      vm.task_queues.m2s.urgent.process();
   }
}
void DovahKitScriptVM::_task_queue::clear() {
   auto  guard = std::lock_guard(this->lock);
   auto& list = this->list;
   //
   for (auto* task : list)
      if (!task->is_blocking() && task->is_fire_and_forget())
         delete task;
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
   QObject::connect(this, &DovahKitScriptVM::scriptEnded, this, [this]() {
      this->_teardown_lua_vm();
      //
      // Delete script-to-main tasks in the case of a script being terminated early, and delete 
      // main-to-script tasks when a script finishes execution for any reason.
      //
      this->task_queues.m2s.normal.clear();
      this->task_queues.m2s.urgent.clear();
      this->task_queues.s2m.clear();
      //
      this->ui_queues.read.clear();
      this->ui_queues.write.clear();
      //
      this->pending_ui_event_count = 0;
   });
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool will_be_flagged) {
      if (!will_be_flagged)
         return;
      auto* message = new editor_script::tasks::m2s::form_deleted;
      message->stub = stub;
      //
      auto  guard   = std::lock_guard(this->task_queues.m2s.urgent.lock);
      auto& list    = this->task_queues.m2s.urgent.list;
      list.push_back(message);
   });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &DovahKitScriptVM::abort);
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
   lua_setfield (this->lua_vm, LUA_REGISTRYINDEX, DovahKitScriptVM::string_format_registry_key);
   lua_pop(this->lua_vm, 1);
   //
   editor_script::expose_form_types_to_lua(this->lua_vm);
   //
   // Prepare API classes:
   //
   #pragma region Queued functions
      lua_newtable(this->lua_vm);
      lua_setfield(this->lua_vm, LUA_REGISTRYINDEX, DovahKitScriptVM::queued_function_registry_key);
   #pragma endregion
   #pragma region Wrapper and listener storage tables
      lua_newtable(this->lua_vm);
      lua_setfield(this->lua_vm, LUA_REGISTRYINDEX, wrapper_storage_registry_key);
      //
      lua_newtable  (this->lua_vm);
      lua_pushstring(this->lua_vm, "v");
      lua_setfield  (this->lua_vm, -2, "__mode");
      lua_setfield(this->lua_vm, LUA_REGISTRYINDEX, wrapper_weakmap_metatable_key);
      //
      lua_newtable(this->lua_vm);
      lua_setfield(this->lua_vm, LUA_REGISTRYINDEX, ui_listener_registry_key);
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
   {  // dovah
      lua_newtable(this->lua_vm); // create a new table
      editor_script::namespace_setup::dovah(this->lua_vm);
      lua_setglobal(this->lua_vm, "dovah"); // assign the new table to a variable
   }
   {  // ui
      lua_newtable(this->lua_vm);
      editor_script::build_all_ui_wrapper_singletons(this->lua_vm);
      lua_setglobal(this->lua_vm, "ui");
   }
   this->pending_ui_event_count = 0;
}
void DovahKitScriptVM::_teardown_lua_vm() {
   auto guard = std::lock_guard(this->exec_lock);
   //
   if (this->lua_vm) {
      lua_close(this->lua_vm);
      this->lua_vm = nullptr;
   }
   //
   for (auto* window : this->widgets.windows) {
      if (!window)
         continue;
      QObject::disconnect(window);
      window->done(-2);
      window->deleteLater();
   }
   this->widgets.windows.clear();
   for (auto* widget : this->widgets.orphans) {
      if (!widget)
         continue;
      QObject::disconnect(widget);
      widget->deleteLater();
   }
   this->widgets.orphans.clear();
   //
   this->pending_ui_event_count = 0;
}

void DovahKitScriptVM::_run_queued_functions() {
   auto start      = lua_gettop(this->lua_vm);
   auto index_list = start + 1;
   auto index_nk   = start + 2;
   auto index_nv   = start + 3;
   if (lua_getfield(this->lua_vm, LUA_REGISTRYINDEX, DovahKitScriptVM::queued_function_registry_key) == LUA_TTABLE) {
      int count = lua_rawlen(this->lua_vm, -1);
      if (!count) {
         lua_settop(this->lua_vm, start);
         return;
      }
      //
      // Clear the list out of the registry (replace it with a blank table), leaving the original list 
      // on the stack for us to use here.
      //
      lua_createtable(this->lua_vm, 0, 0);
      lua_setfield(this->lua_vm, LUA_REGISTRYINDEX, DovahKitScriptVM::queued_function_registry_key);
      //
      // Execute each individual function in the list.
      //
      for (int i = 0; i < count; ++i) {
         lua_geti(this->lua_vm, -1, i + 1);
         editor_script::util::safe_call(this->lua_vm, 0, 0); // this will pop the value
      }
   }
   lua_settop(this->lua_vm, start);
}
void DovahKitScriptVM::_script_thread_loop() {
   editor_script::util::safe_call(this->lua_vm, 0, 0);
   //
   do {
      this->task_queues.s2m.wait_until_empty(); // these can be non-blocking + fire-and-forget
      this->ui_queues.write.wait_until_empty(); // these can be non-blocking + fire-and-forget
      this->_run_queued_functions();
      this->task_queues.m2s.urgent.process();
      this->task_queues.m2s.normal.process();
   } while (this->_should_keep_running());
   //
   this->running = false;
   emit this->scriptEnded(false); // a main-thread handler will catch this and tear down the VM
}

bool DovahKitScriptVM::_should_keep_running() const noexcept {
   if (this->aborted)
      return false;
   //
   // If the script has any script-spawned UI windows open and visible, then this function 
   // should return (true). If we want to be more sophisticated, then we can double-check 
   // that the windows or any controls in them have any event listeners registered.
   //
   // The basic thing we're checking for is, "We're not running script code *right now*, 
   // but can we *end up* running them as a result of any extant event listeners?"
   //
   for (auto* window : this->widgets.windows) {
      if (!window)
         continue;
      if (window->isVisible())
         return true;
   }
   return false;
}

QDialog* DovahKitScriptVM::try_spawn_script_window() noexcept {
   {
      auto guard = std::lock_guard(this->exec_lock);
      if (!this->running)
         return nullptr;
   }
   if (this->widgets.windows.size() >= max_script_windows)
      return nullptr;
   auto* dialog = new QDialog(this->ui_parent);
   dialog->installEventFilter(this);
   this->widgets.windows.push_back(dialog);
   return dialog;
}
void DovahKitScriptVM::set_up_new_scripted_widget(QWidget* widget) {
   widget->installEventFilter(this);
   if (!widget->parentWidget())
      this->accept_new_orphaned_widget(widget);
}
void DovahKitScriptVM::accept_new_orphaned_widget(QWidget* widget) {
   {
      auto guard = std::lock_guard(this->exec_lock);
      if (!this->running)
         return;
   }
   if (!widget)
      return;
   this->widgets.orphans.push_back(widget);
}
void DovahKitScriptVM::widget_no_longer_orphaned(QWidget* widget) {
   {
      auto guard = std::lock_guard(this->exec_lock);
      if (!this->running)
         return;
   }
   if (!widget || !widget->parentWidget())
      return;
   auto& v = this->widgets.orphans;
   v.erase(std::remove(v.begin(), v.end(), widget), v.end());
}
void DovahKitScriptVM::widget_no_longer_referenced(QWidget* widget) {
   if (!widget)
      return;
   if (widget->parentWidget() || qobject_cast<QDialog*>(widget))
      return;
   auto& list = this->widgets.orphans;
   for (auto it = list.begin(); it != list.end(); ++it) {
      if (*it == widget) {
         widget->deleteLater();
         list.erase(it);
         return;
      }
   }
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
   this->ui_queues.read.process();
   this->ui_queues.write.process();
}

bool DovahKitScriptVM::eventFilter(QObject* object, QEvent* event) {
   if (!this->pending_ui_event_count)
      return false;
   return true;
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
      if (!vm.is_aborted()) {
         vm.task_queues.m2s.urgent.process();
      }
      if (vm.is_running() && vm.is_aborted()) {
         luaL_error(vm.lua_vm, "Script terminated at the user's request.");
         __assume(0); // luaL_error performs a jump and so does not return
      }
   }
}
#pragma endregion 

#pragma region DovahKitScriptVMUITaskConduit
void DovahKitScriptVMUITaskConduit::send_message(editor_script::ui_read_task& task) {
   auto& vm = DovahKitScriptVM::get();
   vm.ui_queues.write.wait_until_empty();
   {
      auto  guard = std::lock_guard(vm.ui_queues.read.lock);
      auto& list = vm.ui_queues.read.list;
      list.push_back(&task);
   }
   while (!task.seen)
      if (vm.is_aborted())
         break;
   if (!vm.is_aborted()) {
      vm.task_queues.m2s.urgent.process();
   }
   if (vm.is_running() && vm.is_aborted()) {
      luaL_error(vm.lua_vm, "Script terminated at the user's request.");
      __assume(0); // luaL_error performs a jump and so does not return
   }
}
void DovahKitScriptVMUITaskConduit::send_message(editor_script::cross_thread_task& task) {
   auto& vm = DovahKitScriptVM::get();
   vm.ui_queues.read.wait_until_empty();
   //
   bool blocking = task.is_blocking(); // grab this before adding it to the list, to avoid race conditions (e.g. the main thread executing and deleting a non-blocking task before we get a chance to check)
   {
      auto  guard = std::lock_guard(vm.ui_queues.write.lock);
      auto& list = vm.ui_queues.write.list;
      list.push_back(&task);
   }
   if (blocking) {
      while (!task.seen)
         if (vm.is_aborted())
            break;
      if (!vm.is_aborted()) {
         vm.task_queues.m2s.urgent.process();
      }
      if (vm.is_running() && vm.is_aborted()) {
         luaL_error(vm.lua_vm, "Script terminated at the user's request.");
         __assume(0); // luaL_error performs a jump and so does not return
      }
   }
}
#pragma endregion

#pragma region DovahKitScriptVMPermissionInterface
/*static*/ void DovahKitScriptVMPermissionInterface::verify_form_write_permissions() {
   auto& intfc = DovahKitScriptVMPermissionInterface::get();
   if (false) { // TODO: permission check, when we implement those
      luaL_error(intfc.vm.lua_vm, "The script does not have permission to use APIs that modify form data.");
      __assume(0);
   }
}
/*static*/ void DovahKitScriptVMPermissionInterface::verify_ui_permissions() {
   auto& intfc = DovahKitScriptVMPermissionInterface::get();
   if (false) { // TODO: permission check, when we implement those
      luaL_error(intfc.vm.lua_vm, "The script does not have permission to use APIs related to the UI.");
      __assume(0);
   }
}
#pragma endregion

//
// Given a dovah::form_stub& named stub:
// 
//    __lua_registry[wrapper_storage_registry_key][&stub] == { wrapper, wrapper, wrapper }
//
// Lua allows us to use void pointers as "light userdata," essentially allowing us to use 
// raw pointers as keys or values in Lua tables.
//
#pragma region DovahKitScriptVMUserdataInterface
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
   if (!instance.should_expose_to_script())
      return 0;
   //
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
#pragma endregion

#pragma region DovahKitScriptUIListenerInterface
//
// Event names within this system must be lowercase.
//

namespace {
   const char* signal_name_for_event(const QWidget& widget, const char* event_name) {
      if (qobject_cast<const QPushButton*>(&widget)) {
         if (_stricmp(event_name, "OnActivate") == 0)
            return SIGNAL("clicked");
         if (_stricmp(event_name, "OnCheckStateChange") == 0)
            return SIGNAL("toggled");
      }
      return nullptr;
   }

   template<typename... Args> struct _event_forwarding_lambda {
      _event_forwarding_lambda(QWidget& w, const char* n) : widget(w), event_name(n) {}

      QWidget& widget;
      const std::string event_name;

      void operator()(Args&&... a) {
         //
         // Runs on the main thread.
         //
         DovahKitScriptUIListenerInterface::get().receive_event_from_main_thread(this->widget, this->event_name.c_str(), { std::forward<Args>(args)... });
      }
   };

   void _register_event(QWidget& widget, const char* event_name) {
      //
      // Runs on the script thread.
      //
      auto& vm = DovahKitScriptVM::get();
      if (auto* casted = qobject_cast<QPushButton*>(&widget)) {
         if (_stricmp(event_name, "OnActivate") == 0) {
            QObject::connect(casted, &QPushButton::clicked, &vm, _event_forwarding_lambda(widget, event_name), Qt::ConnectionType::UniqueConnection);
            return;
         }
         if (_stricmp(event_name, "OnCheckStateChange") == 0) {
            QObject::connect(casted, &QPushButton::toggled, &vm, _event_forwarding_lambda(widget, event_name), Qt::ConnectionType::UniqueConnection);
            return;
         }
      }
   }
}

void DovahKitScriptUIListenerInterface::add_listener(QWidget& widget, const char* event_name, const char* listener_name, int listener_index) {
   auto* signal_name = signal_name_for_event(widget, event_name);
   //
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   //
   auto  si_storage = start + 1;
   auto  si_events  = start + 2;
   auto  si_funcs   = start + 3;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, ui_listener_registry_key);
   assert(lua_type(L, -1) == LUA_TTABLE);
   // STACK: - [ ..., storage_root ] +
   lua_pushlightuserdata(L, &widget);
   lua_rawget(L, si_storage);
   // STACK: - [ ..., storage_root, storage_root[&widget] ] +
   if (lua_isnoneornil(L, si_events)) {
      lua_pop(L, 1);
      lua_createtable(L, 0, 1);
      lua_pushlightuserdata(L, &widget);
      lua_pushvalue(L, si_events);
      lua_rawset(L, si_storage);
   }
   // STACK: - [ ..., storage_root, storage_root[&widget] ] +
   lua_getfield(L, si_events, event_name);
   // STACK: - [ ..., storage_root, storage_root[&widget], storage_root[&widget][event_name] ] +
   bool empty = lua_isnoneornil(L, si_funcs);
   if (empty) {
      lua_pop(L, 1);
      lua_createtable(L, 0, 1);
      lua_pushstring(L, event_name);
      lua_pushvalue(L, si_funcs);
      lua_rawset(L, si_events);
      //
      _register_event(widget, event_name);
   }
   lua_pushstring(L, listener_name);
   lua_pushvalue(L, listener_index);
   lua_rawset(L, si_funcs);
   //
   lua_settop(L, start);
}
void DovahKitScriptUIListenerInterface::remove_listener(QWidget& widget, const char* event_name, const char* listener_name) {
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   //
   auto  si_storage = start + 1;
   auto  si_events  = start + 2;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, ui_listener_registry_key); // push 1
   lua_pushlightuserdata(L, &widget);
   lua_rawget(L, si_storage); // STACK: - [ ..., storage_root, storage_root[&widget] ] +
   assert(lua_istable(L, -1));
   lua_copy  (L, -1, si_storage);
   lua_settop(L, si_storage); // STACK: - [ ..., storage_root[&widget] ] +
   lua_getfield(L, si_storage, event_name); // STACK: - [ ..., storage_root[&widget], storage_root[&widget][event_name] ] +
   if (lua_isnoneornil(L, si_events)) { // no events to remove
      lua_settop(L, start);
      return;
   }
   bool empty = true;
   if (listener_name) {
      empty = false;
      lua_pushnil(L);
      lua_setfield(L, si_events, listener_name);
      empty = cobb::lua::isempty(L, si_events);
   }
   if (empty) { // no listeners left for this event. disconnect the Qt signal for it
      lua_pushnil(L);
      lua_setfield(L, si_storage, event_name);
      //
      auto* signal_name = signal_name_for_event(widget, event_name);
      if (signal_name)
         widget.disconnect(signal_name);
   }
   //
   lua_settop(L, start);
}
void DovahKitScriptUIListenerInterface::remove_all_listeners(QWidget& widget) {
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   //
   lua_getfield(L, LUA_REGISTRYINDEX, ui_listener_registry_key);
   lua_pushlightuserdata(L, &widget);
   lua_pushnil(L);
   lua_rawset(L, start + 1);
   //
   widget.disconnect();
   //
   lua_settop(L, start);
}
void DovahKitScriptUIListenerInterface::fire_event(QWidget& widget, const char* event_name, const std::vector<QVariant> params) {
   auto* signal_name = signal_name_for_event(widget, event_name);
   //
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   auto  guard = cobb::lua::set_top_on_exit(L, start);
   //
   auto  si_storage = start + 1;
   auto  si_events  = start + 2;
   auto  si_funcs   = start + 3;
   auto  si_nk      = start + 4;
   auto  si_nv      = start + 5;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, ui_listener_registry_key);
   // STACK: - [ ..., storage ] +
   assert(lua_gettop(L)   == si_storage);
   assert(lua_type(L, -1) == LUA_TTABLE);
   lua_pushlightuserdata(L, &widget);
   if (lua_rawget(L, si_storage) != LUA_TTABLE) {
      --this->vm.pending_ui_event_count;
      return;
   }
   // STACK: - [ ..., storage, storage[&widget] ] +
   assert(lua_gettop(L) == si_events);
   if (lua_getfield(L, si_events, event_name) != LUA_TTABLE) {
      --this->vm.pending_ui_event_count;
      return;
   }
   // STACK: - [ ..., storage, storage[&widget], storage[&widget][event_name] ] +
   lua_pushnil(L); // nk
   while (lua_next(L, si_funcs) != 0) {
      // STACK: - [ ..., storage, storage[&widget], storage[&widget][event_name], key, value ] +
      int argcount = 0;
      for (auto& p : params)
         argcount += cobb::lua::push_qt_variant(L, p);
      editor_script::util::safe_call(L, argcount, 0); // pops (nv), since that's the function
   }
   --this->vm.pending_ui_event_count;
}

void DovahKitScriptUIListenerInterface::receive_event_from_main_thread(QWidget& widget, const char* event_name, const std::vector<QVariant> params) {
   ++this->vm.pending_ui_event_count;
   //
   // Called by the main thread; sends a message to the script thread.
   //
   auto* task  = new editor_script::tasks::m2s::ui_event(widget, event_name, params);
   //
   auto& tq    = DovahKitScriptVM::get().task_queues.m2s.normal;
   auto  guard = std::lock_guard(tq.lock);
   tq.list.push_back(task);
}
#pragma endregion