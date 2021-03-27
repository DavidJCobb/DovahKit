#include "widget.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include <QDialog>
#include <QFrame>

#include "../../cross_thread_tasks/s2m/lambda.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::widget;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      luastackchange_t can_have_layout(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         if (
            self.widget->metaObject() == &QWidget::staticMetaObject // is QWidget and NOT a subclass?
         || qobject_cast<QDialog*>(self.widget)
         || qobject_cast<QFrame*>(self.widget)
         )
            lua_pushboolean(L, true);
         else
            lua_pushboolean(L, false);
         return 1;
      }
   }
   namespace _getters {
      luastackchange_t enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         bool result;
         {
            auto* widget  = self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = widget->isEnabled(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.stub)
            return 0;
         auto* widget  = (QDialog*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         bool  value   = lua_toboolean(L, 2);
         task->handler = [widget, value]() { widget->setEnabled(value); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
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
         QWidget* created = nullptr;
         auto*    task    = new tasks::s2m::lambda(true);
         task->handler = [&created]() {
            created = new QWidget;
            DovahKitScriptVM::get().accept_new_orphaned_widget(created);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         wrapper out;
         auto* mt = wrap_widget(out, created);
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
      { "can_have_layout", &_methods::can_have_layout },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "enabled", &_getters::enabled },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "enabled", &_setters::enabled },
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      editor_script::define_class(L, metatable_key, nullptr, metatable_methods);
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