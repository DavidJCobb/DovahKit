#include "checkbox.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
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
   using namespace dovahscript;
   using cls          = wrappers::ui::checkbox;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      int checked(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QCheckBox::isChecked);
         lua_pushboolean(L, result);
         return 1;
      }
      int state(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         auto result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QCheckBox::checkState);
         switch (result) {
            case Qt::CheckState::Unchecked:
               lua_pushstring(L, "unchecked");
               return 1;
            case Qt::CheckState::PartiallyChecked:
               lua_pushstring(L, "indeterminate");
               return 1;
            case Qt::CheckState::Checked:
               lua_pushstring(L, "checked");
               return 1;
         }
         lua_pushnil(L);
         return 1;
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QCheckBox::text);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      int checked(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property_and_block_signals((wrapped_type*)self.widget, &QCheckBox::setChecked, value);
         return 0;
      }
      int state(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto  value = Qt::CheckState::Unchecked;
         auto* arg   = lua_tostring(L, 2);
         if (_stricmp(arg, "checked") == 0)
            value = Qt::CheckState::Checked;
         else if (_stricmp(arg, "unchecked") == 0)
            value = Qt::CheckState::Unchecked;
         else if (_stricmp(arg, "indeterminate") == 0)
            value = Qt::CheckState::PartiallyChecked;
         else
            cobb::lua::error(L, "`%s` is not a recognized checkbox state", arg);
         api_helpers::set_widget_property_and_block_signals((wrapped_type*)self.widget, &QCheckBox::setCheckState, value);
         return 0;
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QCheckBox::setText, value);
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
            created->setText(text);
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
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "checked", &_getters::checked },
      { "state",   &_getters::state },
      { "text",    &_getters::text },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "checked", &_setters::checked },
      { "state",   &_setters::state },
      { "text",    &_setters::text },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}