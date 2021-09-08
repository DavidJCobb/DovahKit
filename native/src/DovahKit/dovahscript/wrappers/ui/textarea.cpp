#include "textarea.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/warning.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"
#include "../../wrapper.h"

#include "../../tasks/s2m/create_ui_widget.h"
#include "../../tasks/s2m/ui_read_lambda.h"
#include "../../tasks/s2m/ui_write_lambda.h"

#include "../../api_helpers/qt_alignment.h"
#include "../../api_helpers/widget_properties.h"

namespace {
   constexpr int max_allowed_length = 99999;
}

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::textarea;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int clear(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget]() { widget->clear(); };
         send_script_ui_task(*task);
         return 0;
      }
      int redo(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget]() { widget->redo(); };
         send_script_ui_task(*task);
         return 0;
      }
      int select_all(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget]() { widget->selectAll(); };
         send_script_ui_task(*task);
         return 0;
      }
      int undo(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget]() { widget->undo(); };
         send_script_ui_task(*task);
         return 0;
      }
   }
   namespace _getters {
      int max_length(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::maxLength);
         lua_pushnumber(L, result);
         return 1;
      }
      int placeholder(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::placeholderText);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int read_only(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::isReadOnly);
         lua_pushboolean(L, result);
         return 1;
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::toPlainText);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      int max_length(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "max length (integer) expected");
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         cobb::lua::argcheck(L, isnum,      2, "max length (integer) expected");
         cobb::lua::argcheck(L, value >= 0, 2, "the maximum length cannot be negative");
         if (value > max_allowed_length)
            value = max_allowed_length;
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setMaxLength, value);
         return 0;
      }
      int placeholder(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "placeholder (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setPlaceholderText, value);
         return 0;
      }
      int read_only(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setReadOnly, value);
         return 0;
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setPlainText, value);
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         //
         QString text;
         if (lua_gettop(L) > 0) {
            luaL_argcheck(L, lua_isstring(L, 1), 1, "nil or string expected");
            text = QString::fromUtf8(lua_tostring(L, 1));
         }
         //
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
         task->configure = [text](wrapped_type* created) {
            created->setMaxLength(max_allowed_length);
            created->setPlainText(text);
         };
         send_script_ui_task(*task);
         auto* created = task->created;
         delete task;
         //
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
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "clear",      &_methods::clear },
      { "redo",       &_methods::redo },
      { "select_all", &_methods::select_all },
      { "undo",       &_methods::undo },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "max_length",      &_getters::max_length },
      { "placeholder",     &_getters::placeholder },
      { "read_only",       &_getters::read_only },
      { "text",            &_getters::text },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "max_length",       &_setters::max_length },
      { "placeholder",      &_setters::placeholder },
      { "read_only",        &_setters::read_only },
      { "text",             &_setters::text },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}