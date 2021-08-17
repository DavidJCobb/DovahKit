#include "window.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/qt_variant.h"
#include "../../../helpers/qt/layout.h"
#include "../../push_native_object.h"
#include "../../task_reference.h"
#include "../../widget_overrides.h"
#include "../../wrapper.h"
#include "../../core/subsystems/coordinator.h"
#include "../../core/subsystems/events.h"
#include "../../core/subsystems/lifetime.h"
#include "../../core/subsystems/permissions.h"

#include "../../tasks/s2m/create_ui_widget.h"
#include "../../tasks/s2m/lambda.h"
#include "../../tasks/s2m/ui_read_lambda.h"
#include "../../tasks/s2m/ui_write_lambda.h"

#include "../../api_helpers/widget_properties.h"
#include "../../constants/ui_count_limits.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::window;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int hide(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* widget  = (QDialog*) self.widget;
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget]() { widget->done(-1); };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         return 0;
      }
      int show(lua_State* L) {
         static constexpr bool use_blocking_task = false;
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* widget  = (QDialog*) self.widget;
         auto* task    = new tasks::s2m::ui_write_lambda(use_blocking_task);
         task->handler = [widget]() {
            widget->open();
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         if constexpr (use_blocking_task)
            delete task;
         return 0;
      }
   }
   namespace _getters {
      int has_help_button(lua_State* L) {
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
            core::subsystems::coordinator::get().send_ui_read_task(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
      int has_size_handle(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QDialog::isSizeGripEnabled);
         lua_pushboolean(L, result);
         return 1;
      }
      int title(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QDialog::windowTitle);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      int has_help_button(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto  value   = lua_toboolean(L, 2);
         auto* widget  = (wrapped_type*)self.widget;
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, value]() {
            widget->setWindowFlags(widget->windowFlags().setFlag(Qt::WindowContextHelpButtonHint, value));
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         return 0;
      }
      int has_size_handle(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QDialog::setSizeGripEnabled, value);
         return 0;
      }
      int title(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "window title (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QDialog::setWindowTitle, value);
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the ui.widget.new function should not be called with a colon or passed any arguments");
         //
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
         /*
         // If you want to configure the widget, e.g. to provide sensible defaults, you would 
         // do it in this optional handler.
         //
         task->configure = [](wrapped_type* created) {
         };
         */
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         auto* created = task->created;
         delete task;
         //
         if (!created) {
            cobb::lua::error(L, "the script is only allowed to create %d windows", max_script_windows);
         }
         return push_native_object(created);
      }
      int is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace dovahscript::wrappers::ui {
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

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}