#include "window.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"
#include "../../cross_thread_tasks/s2m/spawn_window.h"

#include "helpers/widget_properties.h"

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
         auto* widget  = (QDialog*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget]() { widget->done(-1); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t show(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* widget  = (QDialog*) self.widget;
         auto* task    = new tasks::s2m::lambda(true); // blocking task, just so that the VM doesn't teardown before this gets a chance to run
         task->handler = [widget]() { widget->open(); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t has_help_button(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto* widget  = (wrapped_type*)self.widget;
            task->handler = [widget, &result]() {
               result = widget->windowFlags() & Qt::WindowContextHelpButtonHint;
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t has_size_handle(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QDialog::isSizeGripEnabled);
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t title(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QDialog::windowTitle);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t has_help_button(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto  value   = lua_toboolean(L, 2);
         auto* widget  = (wrapped_type*)self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget, value]() {
            widget->setWindowFlags(widget->windowFlags().setFlag(Qt::WindowContextHelpButtonHint, value));
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t has_size_handle(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QDialog::setSizeGripEnabled, value);
         return 0;
      }
      luastackchange_t title(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "window title (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QDialog::setWindowTitle, value);
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
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace editor_script::wrappers::ui {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "hide", &_methods::hide },
      { "show", &_methods::show },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "has_help_button", &_getters::has_help_button },
      { "has_size_handle", &_getters::has_size_handle },
      { "title",           &_getters::title },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "has_help_button", &_setters::has_help_button },
      { "has_size_handle", &_setters::has_size_handle },
      { "title",           &_setters::title },
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setfield(L, pos, cls::global_name);
   }
}