#include "window.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/spawn_window.h"
#include "../../cross_thread_tasks/s2m/ui_dialog_operation.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::window;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      luastackchange_t hide(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* task = new tasks::s2m::ui_dialog_operation;
         task->target         = (QDialog*) self.widget;
         task->operation      = tasks::s2m::ui_dialog_operation::operation_type::set_visibility;
         task->params.boolean = false;
         DovahKitScriptVMMessenger::get().send_message(task);
         delete task;
         return 0;
      }
      luastackchange_t show(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* task = new tasks::s2m::ui_dialog_operation;
         task->target         = (QDialog*) self.widget;
         task->operation      = tasks::s2m::ui_dialog_operation::operation_type::set_visibility;
         task->params.boolean = true;
         DovahKitScriptVMMessenger::get().send_message(task);
         delete task;
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t has_size_handle(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         //
         auto* task = new tasks::s2m::ui_dialog_operation;
         task->target         = (QDialog*) self.widget;
         task->operation      = tasks::s2m::ui_dialog_operation::operation_type::get_size_grip;
         DovahKitScriptVMMessenger::get().send_message(task);
         lua_pushboolean(L, task->results.boolean);
         delete task;
         //
         return 1;
      }
      luastackchange_t title(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         //
         auto* task = new tasks::s2m::ui_dialog_operation;
         task->target         = (QDialog*) self.widget;
         task->operation      = tasks::s2m::ui_dialog_operation::operation_type::get_title;
         DovahKitScriptVMMessenger::get().send_message(task);
         lua_pushstring(L, task->results.text.toUtf8());
         delete task;
         //
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t has_size_handle(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.stub)
            return 0;
         //
         auto* task = new tasks::s2m::ui_dialog_operation;
         task->target         = (QDialog*) self.widget;
         task->operation      = tasks::s2m::ui_dialog_operation::operation_type::set_size_grip;
         task->params.boolean = lua_toboolean(L, 2);
         DovahKitScriptVMMessenger::get().send_message(task);
         delete task;
         //
         return 0;
      }
      luastackchange_t title(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "window title (string) expected");
         if (!self.widget)
            return 0;
         //
         auto* task = new tasks::s2m::ui_dialog_operation;
         task->target         = (QDialog*) self.widget;
         task->operation      = tasks::s2m::ui_dialog_operation::operation_type::set_title;
         task->params.text    = QString::fromUtf8(lua_tostring(L, 2));
         DovahKitScriptVMMessenger::get().send_message(task);
         delete task;
         //
         return 0;
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.dialog.new function should not be called with a colon or passed any arguments");
         //
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         auto* m = new tasks::s2m::spawn_window;
         DovahKitScriptVMMessenger::get().send_message(m);
         if (m->error) {
            luaL_error(L, "Too many scripted windows already exist.");
         }
         wrapped_type* w = m->result;
         delete m;
         //
         wrapper out;
         auto* mt = wrap_widget(out, w);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t is(lua_State* L) {
         if (cls::check_arg_type(L, 1)) {
            lua_pushboolean(L, 1);
            return 1;
         }
         lua_pushboolean(L, 0);
         return 1;
      }
   }
}

namespace editor_script::wrappers::ui {
   /*static*/ const std::initializer_list<luaL_Reg> window::metatable_methods = {
      { "hide", &_methods::hide },
      { "show", &_methods::show },
   };

   /*static*/ void window::setup(lua_State* L) {
      editor_script::define_class(L, metatable_key, nullptr, metatable_methods);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      lua_setglobal(L, cls::global_name);
   }
   /*static*/ const std::initializer_list<luaL_Reg> window::metatable_getters = {
      { "has_size_handle", &_getters::has_size_handle },
      { "title",           &_getters::title },
   };
   /*static*/ const std::initializer_list<luaL_Reg> window::metatable_setters = {
      { "has_size_handle", &_setters::has_size_handle },
      { "title",           &_setters::title },
   };
}