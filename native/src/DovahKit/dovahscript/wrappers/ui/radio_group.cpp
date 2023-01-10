#include "radio_group.h"
#include <QAbstractButton>
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/warning.h"
#include "../../core/subsystems/events.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"
#include "../../wrapper.h"

#include "../../tasks/s2m/create_button_group.h"
#include "../../tasks/s2m/ui_read_lambda.h"
#include "../../tasks/s2m/ui_write_lambda.h"

#include "../../api_helpers/widget_properties.h"

#include "radio_button.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::radio_group;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int on(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring  (L, 2), 2, "event name (string) expected");
         luaL_argcheck(L, lua_isstring  (L, 3), 3, "listener name (string) expected");
         luaL_argcheck(L, lua_isfunction(L, 4), 4, "listener (function) expected");
         lua_settop(L, 4);
         if (!self.button_group)
            return 0;
         core::subsystems::events::get().add_listener(*self.button_group, lua_tostring(L, 2), lua_tostring(L, 3), 4);
         return 0;
      }
      int remove(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* arg  = wrapper_from_stack<wrappers::ui::radio_button>(L, 2);
         cobb::lua::argcheck(L, arg, 2, "radio_button expected");
         if (!self.button_group)
            return 0;
         if (!arg->widget)
            return 0;
         auto  group   = task_reference(self.button_group);
         auto  button  = task_reference(qobject_cast<QAbstractButton*>(arg->widget));
         auto* task    = new tasks::s2m::ui_write_lambda(true);
         task->handler = [group, button]() {
            group->removeButton(button);
         };
         send_script_ui_task(*task);
         delete task;
         //
         return 0;
      }
      int remove_event_listener(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "event name (string) expected");
         luaL_argcheck(L, lua_isstring(L, 3), 3, "listener name (string) expected");
         lua_settop(L, 3);
         if (!lua_isnoneornil(L, 3))
            luaL_argcheck(L, lua_isstring(L, 3), 3, "listener name (string) expected");
         if (!self.button_group)
            return 0;
         core::subsystems::events::get().remove_listener(*self.button_group, lua_tostring(L, 2), lua_tostring(L, 3));
         return 0;
      }
   }
   namespace _getters {
      int selected_button(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.button_group)
            return 0;
         task_reference<QAbstractButton> result = nullptr;
         {
            auto  group   = task_reference(self.button_group);
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [group, &result]() { result = group->checkedButton(); };
            send_script_ui_task(*task);
            delete task;
         }
         return push_native_object(result);
      }
      int selected_id(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.button_group)
            return 0;
         int result;
         {
            auto  group   = task_reference(self.button_group);
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [group, &result]() { result = group->checkedId(); };
            send_script_ui_task(*task);
            delete task;
         }
         if (result == -1)
            lua_pushnil(L);
         else
            lua_pushinteger(L, result + 1);
         return 1;
      }
   }
   namespace _setters {
      int selected_button(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* arg  = wrapper_from_stack<wrappers::ui::radio_button>(L, 2);
         cobb::lua::argcheck(L, arg, 2, "radio_button expected");
         if (!self.button_group)
            return 0;
         if (!arg->widget)
            return 0;
         auto  group   = task_reference(self.button_group);
         auto  button  = task_reference(qobject_cast<QAbstractButton*>(arg->widget));
         auto* task    = new tasks::s2m::ui_write_lambda(true);
         bool  success = false;
         task->handler = [group, button, &success]() {
            if (group->id(button) != -1) {
               success = true;
               button->setChecked(true);
            }
         };
         send_script_ui_task(*task);
         delete task;
         //
         if (!success)
            luaL_error(L, "this radio group doesn't contain the given radio button");
         //
         return 0;
      }
      int selected_id(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int   id   = -1;
         if (!lua_isnoneornil(L, 2)) {
            int isnum;
            id = lua_tointegerx(L, 2, &isnum);
            luaL_argcheck(L, isnum,   2, "integer expected");
            luaL_argcheck(L, id != 0, 2, "radio button IDs cannot be 0");
            --id; // decrement to go from Lua to zero-indexed
         }
         if (!self.button_group)
            return 0;
         auto  group   = task_reference(self.button_group);
         auto* task    = new tasks::s2m::ui_write_lambda(true);
         bool  success = false;
         task->handler = [group, id, &success]() {
            auto* b = group->button(id);
            if (b) {
               success = true;
               b->setChecked(true);
            }
         };
         send_script_ui_task(*task);
         delete task;
         //
         if (!success)
            cobb::lua::error(L, "this radio group doesn't have a radio button with ID %d", id + 1); // increment to go from zero-indexed to Lua
         //
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the ui.%s.new function should not be called with a colon or passed any arguments", cls::global_name);
         //
         auto* task = new tasks::s2m::create_button_group;
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
      { "on",                    &_methods::on },
      { "remove",                &_methods::remove },
      { "remove_event_listener", &_methods::remove_event_listener },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "selected_button", &_getters::selected_button },
      { "selected_id",     &_getters::selected_id },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "selected_button", &_setters::selected_button },
      { "selected_id",     &_setters::selected_id },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}