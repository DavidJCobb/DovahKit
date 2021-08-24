#include "radio_button.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/warning.h"
#include "../../core/subsystems/lifetime.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"
#include "../../wrapper.h"

#include "../../tasks/s2m/create_ui_widget.h"
#include "../../tasks/s2m/ui_read_lambda.h"
#include "../../tasks/s2m/ui_write_lambda.h"

#include "../../api_helpers/widget_properties.h"

#include "radio_group.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::radio_button;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      int checked(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QRadioButton::isChecked);
         lua_pushboolean(L, result);
         return 1;
      }
      int group(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         auto* result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QRadioButton::group);
         return push_native_object(result);
      }
      int id(lua_State* L) {
         //
         // Within Qt, negative IDs counting down from -2 are used for auto-assigned IDs 
         // when a button is added to a group. Positive IDs are left available for users 
         // to manually assign.
         // 
         // Within Lua, negative IDs counting down from -1 are auto-assigned; positive 
         // IDs counting up from 1 are manually assigned; and 0 is not allowed. We can 
         // smoothly map between the two by adding 1 to convert from Qt to Lua.
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = -1;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto* widget  = (wrapped_type*) self.widget;
            task->handler = [widget, &result]() {
               if (auto* group = widget->group())
                  result = group->id(widget);
            };
            send_script_ui_task(*task);
            delete task;
         }
         if (result == -1)
            lua_pushnil(L);
         else
            lua_pushinteger(L, result + 1);
         return 1;
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QRadioButton::text);
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
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QRadioButton::setChecked, value);
         return 0;
      }
      int group(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         QButtonGroup* group = nullptr;
         QButtonGroup* prior = nullptr;
         if (!lua_isnoneornil(L, 2)) {
            auto* arg = wrapper_from_stack<wrappers::ui::radio_group>(L, 2);
            cobb::lua::argcheck(L, arg, 2, "radio_group expected");
            group = arg->button_group;
         }
         if (!self.widget)
            return 0;
         auto  button  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(true);
         task->handler = [group, button, &prior]() {
            prior = button->group();
            if (prior == group)
               return;
            if (prior)
               prior->removeButton(button);
            if (group)
               group->addButton(button);
         };
         send_script_ui_task(*task);
         delete task;
         //
         if (prior != group) {
            if (prior)
               core::subsystems::lifetime::get().on_hierarchy_bridge_severed(group, self.widget);
         }
         //
         return 0;
      }
      int id(lua_State* L) {
         //
         // Within Qt, negative IDs counting down from -2 are used for auto-assigned IDs 
         // when a button is added to a group. Positive IDs are left available for users 
         // to manually assign.
         // 
         // Within Lua, negative IDs counting down from -1 are auto-assigned; positive 
         // IDs counting up from 1 are manually assigned; and 0 is not allowed. We can 
         // smoothly map between the two by subtracting 1 to convert from Lua to Qt.
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int id = -1;
         if (!lua_isnoneornil(L, 2)) {
            int isnum;
            int id = lua_tointegerx(L, 2, &isnum);
            luaL_argcheck(L, isnum,   2, "expected integer");
            luaL_argcheck(L, id != 0, 2, "radio button IDs cannot be 0");
            --id;
         }
         if (!self.widget)
            return 0;
         auto  widget   = task_reference((wrapped_type*) self.widget);
         auto* task     = new tasks::s2m::ui_write_lambda(true);
         bool  in_group = false;
         bool  id_taken = false;
         task->handler  = [widget, id, &in_group, &id_taken]() {
            auto* group = widget->group();
            if (!group)
               return;
            in_group = true;
            auto* prior = group->button(id);
            if (prior && prior != widget) {
               id_taken = true;
               return;
            }
            group->setId(widget, id);
         };
         send_script_ui_task(*task);
         delete task;
         //
         if (!in_group)
            cobb::lua::error(L, "you must assign the radio button to a radio group before you can set the button's ID");
         if (id_taken)
            cobb::lua::error(L, "the ID %d is already taken by another radio button in this radio button's group", id + 1);
         //
         return 0;
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QRadioButton::setText, value);
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
      { "group",   &_getters::group },
      { "id",      &_getters::id },
      { "text",    &_getters::text },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "checked", &_setters::checked },
      { "group",   &_setters::group },
      { "id",      &_setters::id },
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