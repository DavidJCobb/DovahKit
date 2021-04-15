#include "messaging.h"
#include "userdata.h"
#include "../editor_script_core.h"

#include <array>
#include "../util.h"
#include "../api/allowed_standard_apis.h"
#include "../cross_thread_tasks/_all.h"
#include "../wrappers/_build_metatables.h"
#include "../wrappers/_build_singletons.h"
#include "../class_killer.h"

#include "../../core.h" // needed for DovahKitCore signals
#include "../../form_stub_meta_type.h" // needed for QVariants of form stub pointers

#include "../api/form_type_values.h"
#include "../classes/_all.h"
#include "../../../helpers/lua/qt_variant.h"
#include "../../../helpers/lua/set_top_on_exit.h"
#include "../../../helpers/qt/traversal.h"
#include "../wrapper_util.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QEvent>
#include <QLabel>

#include "../wrappers/form.h" // for the object_is_form function and for internal variant_from_lua
#include "../wrappers/ui/widget.h" // for internal variant_from_lua

#include "../api/namespaces/dovah.h"
#include "../api/namespaces/ui.h"

namespace {
   static constexpr int max_script_windows = 10;
}

namespace {
   void _lua_debug_hook(lua_State* L, lua_Debug* ar) {
      auto& vm = DovahKitScriptVMCore::get();
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
         if (DovahKitScriptVMCore::get().is_aborted()) {
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
   int _wrapper_is_form(lua_State* L) {
      lua_settop(L, 1);
      bool value = false;
      if (lua_type(L, 1) == LUA_TUSERDATA) {
         value = editor_script::cast_to_class(L, 1, editor_script::wrappers::form::metatable_key) != nullptr;
      }
      lua_pushboolean(L, value);
      return 1;
   }
   int _wrapper_is_zombie(lua_State* L) {
      lua_settop(L, 1);
      lua_pushboolean(L, editor_script::userdata_is_zombie(L, 1));
      return 1;
   }
}

void DovahKitScriptVMCore::_task_queue::process(int cap) {
   auto  guard = std::lock_guard(this->lock);
   auto& list  = this->list;
   //
   int count = 0;
   for (auto* task : list) {
      bool blocking = task->is_blocking();
      task->execute();
      if (!blocking && task->is_fire_and_forget())
         delete task;
      if (++count >= cap)
         break;
   }
   list.erase(list.begin(), list.begin() + count);
}
void DovahKitScriptVMCore::_task_queue::wait_until_empty() {
   auto& list = this->list;
   while (!list.empty()) {
   }
   auto& vm = DovahKitScriptVMCore::get();
   if (!vm.is_aborted()) {
      vm.task_queues.m2s.urgent.process();
   }
}
void DovahKitScriptVMCore::_task_queue::clear() {
   auto  guard = std::lock_guard(this->lock);
   auto& list = this->list;
   //
   for (auto* task : list)
      if (!task->is_blocking() && task->is_fire_and_forget())
         delete task;
   list.clear();
}

#pragma region DovahKitScriptVM
DovahKitScriptVMCore::DovahKitScriptVMCore() {
   this->main_thread_tick_timer.setSingleShot(false);
   this->main_thread_tick_timer.setInterval(0);
   QObject::connect(this, &DovahKitScriptVMCore::scriptEnded, this, [this]() { this->main_thread_tick_timer.stop(); });
   //
   QObject::connect(&this->main_thread_tick_timer, &QTimer::timeout, this, &DovahKitScriptVMCore::mainThreadLoop);
   QObject::connect(this, &DovahKitScriptVMCore::scriptEnded, this, [this]() {
      this->_teardown_lua_vm();
      //
      // Delete script-to-main tasks in the case of a script being terminated early, and delete 
      // main-to-script tasks when a script finishes execution for any reason.
      //
      this->ui_queues.events.clear();
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
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &DovahKitScriptVMCore::abort);
}
DovahKitScriptVMCore::~DovahKitScriptVMCore() {
   this->abort();
   if (this->thread.joinable()) // even if it's finished running, we need to join it or std::thread::operator= below will break
      this->thread.join();
   this->_teardown_lua_vm();
   this->running = false;
}

void DovahKitScriptVMCore::_setup_lua_vm() {
   assert(this->ui_model_observers.pointers.empty());
   assert(this->widgets.extant_widget_count == 0);
   assert(this->pending_ui_event_count == 0);
   //
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
   {  // object_is_form
      auto ti = lua_gettop(this->lua_vm);
      lua_pushstring   (this->lua_vm, "object_is_form");
      lua_pushcfunction(this->lua_vm, &_wrapper_is_form);
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
   lua_setfield (this->lua_vm, LUA_REGISTRYINDEX, DovahKitScriptVMCore::string_format_registry_key);
   lua_pop(this->lua_vm, 1);
   //
   editor_script::expose_form_types_to_lua(this->lua_vm);
   //
   // Prepare API classes:
   //
   #pragma region Queued functions
      lua_newtable(this->lua_vm);
      lua_setfield(this->lua_vm, LUA_REGISTRYINDEX, ui_locked_queue_registry_key);
      lua_newtable(this->lua_vm);
      lua_setfield(this->lua_vm, LUA_REGISTRYINDEX, ui_unlocked_queue_registry_key);
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
      editor_script::namespace_setup::ui(this->lua_vm);
      editor_script::build_all_ui_wrapper_singletons(this->lua_vm);
      lua_setglobal(this->lua_vm, "ui");
   }
   this->pending_ui_event_count = 0;
   this->ui_lock_override       = ui_lock_override_state::unchanged;
}
namespace {
   template<typename T> requires std::is_base_of_v<QObject, T> static void _teardown_list_helper(QVector<T*>& list) {
      for (auto* object : list) {
         if (!object)
            continue;
         QObject::disconnect(object, nullptr, nullptr, nullptr);
         if constexpr (std::is_same_v<QDialog, T>) {
            object->done(-2);
         }
         object->deleteLater();
      }
      list.clear();
   }
}
void DovahKitScriptVMCore::_teardown_lua_vm() {
   auto guard = std::lock_guard(this->running);
   
   if (auto* L = this->lua_vm) {
      this->lua_vm = nullptr;
      lua_close(L);
   }
   
   _teardown_list_helper(this->widgets.windows);
   _teardown_list_helper(this->widgets.orphans.widgets);
   _teardown_list_helper(this->widgets.orphans.button_groups);
   //
   // The next two calls are needed if the script ran from start to finish: if a script shows a dialog box 
   // and then reaches its end, then technically, any variables that aren't up-values for event listeners 
   // will go out of scope, widgets included. The reason they're not summarily deleted is because we don't 
   // delete unreferenced widgets if they're in a visible window... but if the script ended because the 
   // user closed the last scripted window...
   //
   _teardown_list_helper(this->widgets.pending_deletion.widgets);
   _teardown_list_helper(this->widgets.pending_deletion.button_groups);
   this->widgets.extant_widget_count = 0;
   
   for (auto* o : this->ui_model_observers.pointers)
      delete o;
   this->ui_model_observers.pointers.clear();
   this->ui_model_observers.refcounts.clear();
   this->pending_ui_event_count = 0;
}

void DovahKitScriptVMCore::_run_queued_functions(bool ui_locked) {
   auto start      = lua_gettop(this->lua_vm);
   auto index_list = start + 1;
   auto index_nk   = start + 2;
   auto index_nv   = start + 3;
   //
   this->ui_lock_override = ui_locked ? ui_lock_override_state::locked : ui_lock_override_state::unlocked;
   //
   auto* key = ui_locked ? ui_locked_queue_registry_key : ui_unlocked_queue_registry_key;
   auto* L   = this->lua_vm;
   if (lua_getfield(L, LUA_REGISTRYINDEX, key) == LUA_TTABLE) {
      int count = lua_rawlen(L, -1);
      if (count) {
         //
         // Clear the list out of the registry (replace it with a blank table), leaving the original list 
         // on the stack for us to use here.
         //
         lua_createtable(L, 0, 0);
         lua_setfield(L, LUA_REGISTRYINDEX, key);
         //
         // Execute each individual function in the list.
         //
         for (int i = 0; i < count; ++i) {
            lua_geti(L, -1, i + 1); // get the function
            editor_script::util::safe_call(this->lua_vm, 0, 0); // this will pop the function
         }
      }
   }
   lua_settop(this->lua_vm, start);
   //
   this->ui_lock_override = DovahKitScriptVMCore::ui_lock_override_state::unchanged;
}
void DovahKitScriptVMCore::_script_thread_loop() {
   editor_script::util::safe_call(this->lua_vm, 0, 0);
   //
   do {
      this->task_queues.s2m.wait_until_empty(); // these can be non-blocking + fire-and-forget
      this->ui_queues.write.wait_until_empty(); // these can be non-blocking + fire-and-forget
      this->task_queues.m2s.urgent.process();
      this->_run_queued_functions(false);
      {
         auto& pd = this->widgets.pending_deletion.widgets;
         for (auto* widget : pd) {
            this->ui_queues.events.forget_about(*widget); // gotta do this before events are processed. since we sever a widget's signals when we mark it for deletion, we don't have to worry about it generating more events later
            widget->deleteLater();
         }
         pd.clear();
      }
      {
         auto& pd = this->widgets.pending_deletion.button_groups;
         for (auto* group : pd) {
            group->deleteLater();
         }
         pd.clear();
      }
      this->ui_queues.events.process();
      this->_run_queued_functions(true);
   } while (this->_should_keep_running());
   //
   this->main_thread_tick_timer.stop();
   this->running = false;
   emit this->scriptEnded(false); // a main-thread handler will catch this and tear down the VM
}

bool DovahKitScriptVMCore::_should_keep_running() const noexcept {
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

/*static*/ void DovahKitScriptVMCore::require_script_thread() {
   assert(std::this_thread::get_id() == DovahKitScriptVMCore::get().thread.get_id());
}
/*static*/ void DovahKitScriptVMCore::require_client_thread() {
   assert(std::this_thread::get_id() != DovahKitScriptVMCore::get().thread.get_id());
}
/*static*/ void DovahKitScriptVMCore::require_wrapper_teardown_thread() {
   //
   // Wrappers should always be torn down on the script thread, UNLESS we are tearing down 
   // the entire VM, which (by necessity) happens on the main thread. In the latter case, 
   // the main thread basically "owns" the Lua VM.
   //
   // The very first part of the entire-VM-teardown process is to grab the Lua state pointer 
   // locally and then write nullptr to the original pointer, so that nothing else can get 
   // to the state. Conveniently, that's something we can check for here.
   //
   auto& d = DovahKitScriptVMCore::get();
   if (d.lua_vm)
      assert(std::this_thread::get_id() == d.thread.get_id());
   else
      assert(std::this_thread::get_id() != DovahKitScriptVMCore::get().thread.get_id());
}

//
// Widgets should be deleted when all of the following conditions are met:
//
//  - The widget, and all other widgets in its containing hierarchy, are no longer referred 
//    to by any Lua script variables.
//
//  - If the widget is or is inside of a window, that window is hidden.
//
//  - If any of the widgets in the containing hierarchy are QAbstractButton instances, and 
//    if any of those widgets are part of a QButtonGroup, then all of those QButtonGroups 
//    must also no longer be referred to by any Lua script variable.
//
//  - If any of the widgets in the containing hierarchy are QAbstractButton instances, and 
//    if any of those widgets are part of a QButtonGroup, then the above constraints must 
//    also be met for every other button in all of those QButtonGroups.
//
// If those conditions are met, then the hierarchies and their contained widgets and button 
// groups shall be considered "abandoned."
//
// Note that because many UI operations are fire-and-forget and do not block the script 
// thread, it's possible for a widget to be "rescued" from deletion after it has been 
// marked for deletion. Refer to (DovahKitScriptVMCore::widget_no_longer_orphaned) and its 
// code comments for further information.
//
namespace {
   // Returns false if it discovers that any widget or button group in the hierarchy being tested 
   // is referenced by Lua.
   bool _find_abandoned_helper(QWidget* basis, QList<QWidget*>& widgets, QList<QButtonGroup*>& groups, int& all_widgets_count) {
      auto* root = cobb::qt::topmost_container_of(basis);
      if (auto* dialog = qobject_cast<QDialog*>(root)) // QWidget::parentWindow just traverses upward. no need to do it twice
         if (dialog->isVisible()) // visible windows and their contents should never be considered abandoned
            return false;
      if (widgets.contains(root))
         return true;
      auto& ud_brain = DovahKitScriptVMUserdataInterface::get();
      //
      int found = 0;
      for (auto* o : root->children()) {
         auto* w = qobject_cast<QWidget*>(o);
         if (!w)
            continue;
         ++found;
         if (auto* button = qobject_cast<QAbstractButton*>(w)) {
            if (auto* g = button->group()) {
               if (ud_brain.wrapper_exists_for(g))
                  return false;
               if (!groups.contains(g))
                  groups.push_back(g);
            }
         }
         if (ud_brain.wrapper_exists_for(w))
            return false;
      }
      all_widgets_count += found;
      widgets.push_back(root);
      return true;
   }
}
// Given a basis widget, searches the widget's entire containing hierarchy as well as any 
// containing hierarchies linked by a QButtonGroup, and returns true if all of the searched 
// hierarchies are abandoned by Lua script.
bool DovahKitScriptVMCore::find_abandoned_widgets_and_groups(QObject* basis, QList<QWidget*>& widgets, QList<QButtonGroup*>& groups, int& all_widgets_count) {
   widgets.clear();
   groups.clear();
   all_widgets_count = 0; // number of all widgets in all found hierarchies, if the hierarchies are all abandoned
   //
   QList<QWidget*>      wl;
   QList<QButtonGroup*> gl;
   int found = 0;
   if (auto* bw = qobject_cast<QWidget*>(basis)) {
      if (!_find_abandoned_helper(bw, wl, gl, found))
         return false;
   } else if (auto* bg = qobject_cast<QButtonGroup*>(basis)) {
      gl = { bg };
   } else {
      assert(false && "unsupported QObject type"));
   }
   while (!gl.isEmpty()) {
      QList<QButtonGroup*> next_pass;
      for (auto* g : gl) {
         for (auto* w : g->buttons()) {
            if (!_find_abandoned_helper(w, wl, next_pass, found))
               return false;
         }
      }
      groups.append(gl);
      gl = next_pass;
   }
   widgets = wl;
   groups  = gl;
   all_widgets_count = found;
   return true;
}
void DovahKitScriptVMCore::mark_abandoned_hierarchy_for_delete(const QList<QWidget*>& abandoned_roots, const QList<QButtonGroup*>& abandoned_groups, int all_widgets_count) {
   {
      auto& orphans = this->widgets.orphans.widgets;
      auto& pending = this->widgets.pending_deletion.widgets;
      for (auto* root : abandoned_roots) {
         int i = orphans.indexOf(root);
         assert(i >= 0);
         #if _DEBUG
            int  children = root->children().size();
            auto name     = root->objectName();
            __debugbreak();
         #endif
         //
         // Sever all signal/slot connections to the condemned widgets.
         //
         cobb::qt::for_each_widget_in_hierarchy(root, [](QWidget* current) {
            QObject::disconnect(current, nullptr, nullptr, nullptr); // these arguments are needed to distinguish a static function call from a call-super
            return false;
         });
         //
         // Queue the root widget for deletion. We can't use QWidget::deleteLater immediately, 
         // because there may be already-received UI events pertaining to this widget that are 
         // about to execute.
         //
         pending.push_back(root);
         orphans.remove(i);
      }
   }
   this->widgets.extant_widget_count -= all_widgets_count;
   {
      auto& orphans = this->widgets.orphans.button_groups;
      auto& pending = this->widgets.pending_deletion.button_groups;
      for (auto* group : abandoned_groups) {
         int i = orphans.indexOf(group);
         assert(i >= 0);
         //
         QObject::disconnect(group, nullptr, nullptr, nullptr);
         pending.push_back(group);
         orphans.remove(i);
      }
   }
}

QDialog* DovahKitScriptVMCore::try_spawn_script_window() noexcept {
   DovahKitScriptVMCore::require_client_thread();
   if (!this->running)
      return nullptr;
   if (this->widgets.windows.size() >= max_script_windows)
      return nullptr;
   auto* dialog = new QDialog(this->ui_parent);
   dialog->installEventFilter(this);
   this->widgets.windows.push_back(dialog);
   ++this->widgets.extant_widget_count;
   return dialog;
}
void DovahKitScriptVMCore::set_up_new_scripted_widget(QWidget* widget) {
   DovahKitScriptVMCore::require_client_thread();
   widget->installEventFilter(this);
   if (auto* label = qobject_cast<QLabel*>(widget)) {
      QObject::connect(label, &QLabel::linkActivated, [label](const QString& url) {
         emit DovahKitScriptVM::get().userClickedLink(url, label->window());
      });
   }
   ++this->widgets.extant_widget_count;
   if (!widget->parentWidget())
      this->accept_new_orphaned_widget(widget);
}
void DovahKitScriptVMCore::accept_new_orphaned_widget(QWidget* widget) {
   DovahKitScriptVMCore::require_client_thread();
   if (!this->running)
      return;
   if (!widget)
      return;
   this->widgets.orphans.widgets.push_back(widget);
}
void DovahKitScriptVMCore::widget_no_longer_orphaned(QWidget* widget) {
   DovahKitScriptVMCore::require_client_thread();
   if (!this->running)
      return;
   if (!widget || !widget->parentWidget())
      return;
   auto& v = this->widgets.orphans.widgets;
   v.erase(std::remove(v.begin(), v.end(), widget), v.end());
   //
   // Consider the following code:
   //
   //    local child = ui.button.new()
   //    do
   //       local parent = ui.widget.new()
   //       parent:add_child(child)
   //    end
   //    collectgarbage("collect")
   //    collectgarbage("collect")
   //
   // Because most of our UI write operations are fire-and-forget and asynchronous (i.e. they 
   // do not block the Lua script), it's possible for the above code snippet to force garbage 
   // collection after (parent) goes out of scope, but before the (parent:add_child) call is 
   // actually acted on. This will cause (DovahKitScriptVMCore::widget_no_longer_referenced) 
   // to process the parent before the child element is added to the parent.
   //
   // Now, at that point, the parent belongs to a widget hierarchy consisting only of itself, 
   // and the parent is unreferenced, so it will be queued for deletion. If everything was 
   // happening one thing at a time, then the child widget would've been appended to the 
   // parent and, by virtue of still being referred to by Lua, would've rescued the parent 
   // from deletion; but since the child hasn't actually been appended yet, that doesn't 
   // occur. Barring any further intervention, the parent will be marked for deletion before 
   // the child is appended to it, and then deleted after the child is appended (due to the 
   // order in which we process cross-thread tasks), which in turn results in the child being 
   // deleted out from under Lua, causing a crash when it is next accessed.
   //
   // So let's intervene! We'll make it so that if a Lua widget ceases to be orphaned -- that 
   // is, if it's appended to another widget hierarchy -- then it can rescue that hierarchy 
   // from deletion:
   //
   auto* root = cobb::qt::topmost_container_of(widget);
   assert(root && root != widget);
   auto& pd = this->widgets.pending_deletion.widgets;
   auto& ow = this->widgets.orphans.widgets;
   int   i  = pd.indexOf(root);
   if (i >= 0) {
      pd.remove(i);
      ow.push_back(root);
   }
}
void DovahKitScriptVMCore::widget_no_longer_referenced(QWidget* widget) {
   DovahKitScriptVMCore::require_wrapper_teardown_thread(); // caller should be wrapper::teardown via wrapper __gc
   if (!widget)
      return;
   if (!this->lua_vm) // teardown in progress; we will delete everything as part of that process
      return;
   //
   // In general, we want to delete widgets that have been abandoned by the script, and 
   // we're notified when any single widget is no longer referred to by a script variable. 
   // The naive approach, then, would be to queue a widget for deletion if, at the time 
   // that it becomes unreferenced, it has no parent. However, doing things that way will 
   // cause code like this to cause a crash --
   //
   //    local child = ui.button.new()
   //    child.text = "Test!"
   //    do
   //       local parent = ui.parent.new()
   //       parent:add_child(child)
   //    end
   //    collectgarbage("collect")
   //    collectgarbage("collect") -- calling this twice in a row is deliberate
   //
   //    local window = ui.window.new()
   //    local button = ui.button.new("Check child")
   //    window:set_layout("grid")
   //    window:add_child(button)
   //    button:on("OnActivated", "", function()
   //       dovah.log_message(child.text) -- crash here!
   //    end)
   //    window:show()
   //
   // -- because the "parent" widget will be deleted, causing its descendant widgets to 
   // be deleted as well, such that the Lua wrapper for the "child" widget is left with 
   // a dangling pointer. The solution to this is to delete widgets only when the entire 
   // hierarchy to which they belong is unreferenced by Lua.
   //
   // Refer to (DovahKitScriptVMCore::widget_no_longer_orphaned) to read about another 
   // important dimension to this problem that we mitigate there.
   //
   QList<QWidget*>      abandoned_roots;
   QList<QButtonGroup*> abandoned_groups;
   int count = 0;
   if (!this->find_abandoned_widgets_and_groups(widget, abandoned_roots, abandoned_groups, count))
      return;
   this->mark_abandoned_hierarchy_for_delete(abandoned_roots, abandoned_groups, count);
}

QButtonGroup* DovahKitScriptVMCore::try_spawn_button_group() {
   DovahKitScriptVMCore::require_client_thread();
   auto* group = new QButtonGroup(this);
   this->widgets.orphans.button_groups.push_back(group);
   return group;
}
void DovahKitScriptVMCore::button_group_lost_a_member(QButtonGroup* group) {
   DovahKitScriptVMCore::require_script_thread();
   if (DovahKitScriptVMUserdataInterface::get().wrapper_exists_for(group))
      return;
   QList<QWidget*>      abandoned_roots;
   QList<QButtonGroup*> abandoned_groups;
   int count = 0;
   if (!this->find_abandoned_widgets_and_groups(group, abandoned_roots, abandoned_groups, count))
      return;
   this->mark_abandoned_hierarchy_for_delete(abandoned_roots, abandoned_groups, count);
}
void DovahKitScriptVMCore::button_group_gained_a_member(QButtonGroup* group) {
   DovahKitScriptVMCore::require_script_thread();
   auto& pd = this->widgets.pending_deletion.button_groups;
   auto& og = this->widgets.orphans.button_groups;
   int   i  = pd.indexOf(group);
   if (i >= 0) {
      pd.remove(i);
      og.push_back(group);
   }
}
void DovahKitScriptVMCore::button_group_no_longer_referenced(QButtonGroup* group) {
   DovahKitScriptVMCore::require_wrapper_teardown_thread(); // caller should be wrapper::teardown via wrapper __gc
   if (!group)
      return;
   if (!this->lua_vm) // teardown in progress; we will delete everything as part of that process
      return;
   QList<QWidget*>      abandoned_roots;
   QList<QButtonGroup*> abandoned_groups;
   int count = 0;
   if (!this->find_abandoned_widgets_and_groups(group, abandoned_roots, abandoned_groups, count))
      return;
   this->mark_abandoned_hierarchy_for_delete(abandoned_roots, abandoned_groups, count);
}

void DovahKitScriptVMCore::model_observer_reference_gained(ObservableStandardItemModelObserver* observer) {
   DovahKitScriptVMCore::require_script_thread();
   if (!observer)
      return;
   auto& store = this->ui_model_observers;
   auto& p_list = store.pointers;
   auto& c_list = store.refcounts;
   //
   auto it = std::find(p_list.begin(), p_list.end(), observer);
   if (it != p_list.end()) {
      auto i = it - p_list.begin();
      ++c_list[i];
      return;
   }
   //
   p_list.push_back(observer);
   c_list.push_back(1);
}
void DovahKitScriptVMCore::model_observer_reference_lost(ObservableStandardItemModelObserver* observer) {
   DovahKitScriptVMCore::require_wrapper_teardown_thread(); // caller should be wrapper::teardown via wrapper __gc
   if (!observer)
      return;
   auto& store  = this->ui_model_observers;
   auto& p_list = store.pointers;
   auto& c_list = store.refcounts;
   //
   auto it = std::find(p_list.begin(), p_list.end(), observer);
   assert(it != p_list.end());
   auto i = it - p_list.begin();
   //
   if (--c_list[i] > 0)
      return;
   assert(c_list[i] == 0 && "How is the refcount negative?!");
   //
   // You'd expect that we'd destroy an unreferenced observer now, right? But nah. See, this 
   // function runs on the script thread, but we can only safely (un)register observers on 
   // the main thread. We'll prune zero-refcount observers on the main thread.
   //
   // As a bonus, we don't have to worry about invalidating iterators if, say, we loop over 
   // model observers elsewhere and do some task (e.g. zombifying some of them) that causes 
   // some of them to become unreferenced within Lua.
   //
}
void DovahKitScriptVMCore::zombify_all_invalid_model_observers() {
   DovahKitScriptVMCore::require_script_thread();
   auto& ud_brain = DovahKitScriptVMUserdataInterface::get();
   for (auto* o : this->ui_model_observers.pointers) {
      if (!o)
         continue;
      if (o->isValid())
         continue;
      ud_brain.remove_model_observer(*o);
   }
}

void DovahKitScriptVMCore::queue_lua_function(int stack_pos, bool lock_ui_for_function) {
   DovahKitScriptVMCore::require_script_thread();
   auto* L   = this->lua_vm;
   auto* key = lock_ui_for_function ? ui_locked_queue_registry_key : ui_unlocked_queue_registry_key;
   //
   stack_pos = lua_absindex(L, stack_pos);
   lua_getfield(L, LUA_REGISTRYINDEX, key);
   auto storage = lua_gettop(L);
   assert(lua_type(L, -1) == LUA_TTABLE);
   //
   lua_pushinteger(L, lua_rawlen(L, storage) + 1); // key to write to
   lua_pushvalue(L, stack_pos);
   lua_rawset(L, storage); // pops value and key
   //
   lua_pop(L, 1); // pop storage
}

int DovahKitScriptVMCore::push_to_lua(const QVariant& p) {
   DovahKitScriptVMCore::require_script_thread();
   auto* L  = this->lua_vm;
   auto  ut = p.userType();
   if (ut == qMetaTypeId<dovah::form_stub*>()) { // the generic helper functions can't handle any DovahKit-specific types
      editor_script::wrapper out;
      auto* mt = wrap_form(out, p.value<dovah::form_stub*>());
      return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
   } else if (ut == QMetaType::QObjectStar) {
      if (auto* widget = qobject_cast<QWidget*>(p.value<QObject*>())) {
         editor_script::wrapper out;
         auto* mt = wrap_widget(out, widget);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      return 0;
   }
   return cobb::lua::push_qt_variant(L, p);
}
QVariant DovahKitScriptVMCore::variant_from_lua(int stack_pos) {
   DovahKitScriptVMCore::require_script_thread();
   auto* L = this->lua_vm;
   if (lua_gettop(L) < stack_pos)
      return QVariant();
   if (lua_type(L, stack_pos) != LUA_TUSERDATA)
      return cobb::lua::to_qt_variant(L, stack_pos);
   //
   if (auto* w = (editor_script::wrapper*) editor_script::cast_to_class(L, stack_pos, editor_script::wrappers::form::metatable_key)) {
      if (w->depth == 0 && !w->is_collection)
         return QVariant::fromValue<dovah::form_stub*>(w->stub);
      return QVariant();
   }
   if (auto* w = (editor_script::wrapper*) editor_script::cast_to_class(L, stack_pos, editor_script::wrappers::ui::widget::metatable_key)) {
      return QVariant::fromValue<QObject*>(w->widget);
   }
   //
   return QVariant();
}

void DovahKitScriptVMCore::abort() {
   auto guard = std::lock_guard(this->running);
   if (this->running) {
      this->aborted = true;
      this->task_queues.s2m.clear();
      this->ui_queues.read.clear();
      this->ui_queues.write.clear();
   }
}
void DovahKitScriptVMCore::runScript(const QString& code, const QString& name) {
   auto guard = std::lock_guard(this->running);
   if (this->running)
      return;
   if (this->thread.joinable()) // even if it's finished running, we need to join it or std::thread::operator= below will break
      this->thread.join();
   auto& facade = DovahKitScriptVM::get();
   this->aborted = false;
   this->running = true;
   this->main_thread_tick_timer.start();
   emit facade.scriptStarted();
   this->_teardown_lua_vm();
   this->_setup_lua_vm();
   //
   auto buffer = code.toUtf8();
   auto result = luaL_loadbufferx(this->lua_vm, buffer.data(), buffer.size(), name.toUtf8().data(), "t"); // equivalent to (lua_load) with a built-in lua_Reader
   if (result == LUA_OK) {
      this->thread = std::thread(&DovahKitScriptVMCore::_script_thread_loop, this);
      return;
   }
   //
   // If something went wrong:
   //
   switch (result) {
      case LUA_ERRMEM:
      case LUA_ERRSYNTAX:
      default:
         auto message = QString::fromUtf8(lua_tostring(this->lua_vm, -1));
         emit DovahKitScriptVMCore::get().messageLogged(message);
         break;
   }
   this->_teardown_lua_vm();
   this->main_thread_tick_timer.stop();
   this->running = false;
   emit scriptEnded(true);
}

void DovahKitScriptVMCore::setUIParentWidget(QWidget* widget) {
   auto guard = std::lock_guard(this->running);
   if (!this->running)
      this->ui_parent = widget;
}

void DovahKitScriptVMCore::mainThreadLoop() {
   {
      auto& store  = this->ui_model_observers;
      auto& p_list = store.pointers;
      auto& c_list = store.refcounts;
      //
      // Code here is basically mimicking the erase-remove idiom:
      //
      size_t size = c_list.size();
      size_t read = 0;
      size_t next = 0;
      for (; read < size; ++read) {
         if (c_list[read]) {
            if (read != next) {
               c_list[next] = c_list[read];
               p_list[next] = std::move(p_list[read]);
            }
            ++next;
         } else {
            delete p_list[read];
            p_list[read] = nullptr;
         }
      }
      c_list.resize(next);
      p_list.resize(next);
   }
   this->task_queues.s2m.process();
   this->ui_queues.read.process();
   this->ui_queues.write.process();
}

bool DovahKitScriptVMCore::eventFilter(QObject* object, QEvent* event) {
   switch (event->type()) { // events that we don't want to ever block (and we can get away with that because we also don't send these to Lua)
      //
      // Blocking some of these events can cause the UI to fail to react to them properly; 
      // for example, if the script sets a widget's enable state while we're blocking the 
      // EnabledChange event, then the widget won't visually update until the user does 
      // something to update it (e.g. mouseover). We don't want that.
      //
      // Not sure which of these events actually cause things like that to happen, versus 
      // which are just bare notifications. Not sure I need to care, either.
      //
      case QEvent::ChildAdded:
      case QEvent::ChildRemoved:
      case QEvent::Close:
      case QEvent::CursorChange:          // a widget's desired cursor graphic has changed
      case QEvent::DeferredDelete:
      case QEvent::EnabledChange:         // a widget's enable state has changed
      case QEvent::Expose:
      case QEvent::FontChange:            // a widget's font has changed
      case QEvent::Hide:                  // a widget was hidden
      case QEvent::LanguageChange:        // the program's translation changed
      case QEvent::LayoutDirectionChange: // layout update
      case QEvent::LayoutRequest:         // layout update
      case QEvent::LocaleChange:          // the system locale has changed
      case QEvent::OrientationChange:     // the screen orientation has changed
      case QEvent::Paint:                 // screen repaint needed
      case QEvent::PaletteChange:         // a widget's palette has changed
      case QEvent::ParentAboutToChange:   // a widget is about to be repainted
      case QEvent::ParentChange:          // a widget has been repainted
      case QEvent::ReadOnlyChange:        // a widget's read-only state has changed
      case QEvent::ScrollPrepare:
      case QEvent::Show:                  // a widget was shown
      case QEvent::ShowToParent:          // a child widget was shown
      case QEvent::StatusTip:             // a status bar tip was shown
      case QEvent::StyleChange:           // a widget's style has changed
      case QEvent::ThreadChange:          // a widget was moved across threads
      case QEvent::ToolTip:               // a widget's tooltip was shown
      case QEvent::ToolTipChange:         // a widget's tooltip changed
      case QEvent::UpdateLater:
      case QEvent::UpdateRequest:         // a widget needs to be repainted
      case QEvent::WindowDeactivate:      // a window was deactivated
      case QEvent::WindowStateChange:     // a window was minimized or maximized
      case QEvent::WindowTitleChange:     // a window's title changed
      case QEvent::WinIdChange:
      case QEvent::ZOrderChange:
         return false;
   }
   if (this->ui_lock_override != ui_lock_override_state::unchanged)
      return this->ui_lock_override == ui_lock_override_state::locked;
   if (!this->pending_ui_event_count)
      return false;
   return true;
}
#pragma endregion