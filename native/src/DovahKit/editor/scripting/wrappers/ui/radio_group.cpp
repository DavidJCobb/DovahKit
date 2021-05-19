#include "radio_group.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/ui_listeners.h"
#include "../../systems/userdata.h"

#include <QAbstractButton>
#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"

#include "radio_button.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::radio_group;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      luastackchange_t on(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring  (L, 2), 2, "event name (string) expected");
         luaL_argcheck(L, lua_isstring  (L, 3), 3, "listener name (string) expected");
         luaL_argcheck(L, lua_isfunction(L, 4), 4, "listener (function) expected");
         lua_settop(L, 4);
         if (!self.button_group)
            return 0;
         DovahKitScriptUIListenerInterface::get().add_listener(*self.button_group, lua_tostring(L, 2), lua_tostring(L, 3), 4);
         return 0;
      }
      luastackchange_t remove(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* arg  = wrapper_from_stack<editor_script::wrappers::ui::radio_button>(L, 2);
         luaL_argcheck(L, arg, 2, "radio_button expected");
         if (!self.button_group)
            return 0;
         if (!arg->widget)
            return 0;
         auto* group   = self.button_group;
         auto* button  = qobject_cast<QAbstractButton*>(arg->widget);
         auto* task    = new tasks::s2m::lambda(true);
         task->handler = [group, button]() {
            group->removeButton(button);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         return 0;
      }
      luastackchange_t remove_event_listener(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "event name (string) expected");
         luaL_argcheck(L, lua_isstring(L, 3), 3, "listener name (string) expected");
         lua_settop(L, 3);
         if (!lua_isnoneornil(L, 3))
            luaL_argcheck(L, lua_isstring(L, 3), 3, "listener name (string) expected");
         if (!self.button_group)
            return 0;
         DovahKitScriptUIListenerInterface::get().remove_listener(*self.button_group, lua_tostring(L, 2), lua_tostring(L, 3));
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t selected_button(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.button_group)
            return 0;
         QAbstractButton* result = nullptr;
         {
            auto* group   = self.button_group;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [group, &result]() { result = group->checkedButton(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (!result)
            return 0;
         wrapper out;
         auto* mt = wrap_widget(out, result);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t selected_id(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.button_group)
            return 0;
         int result;
         {
            auto* group   = self.button_group;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [group, &result]() { result = group->checkedId(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
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
      luastackchange_t selected_button(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* arg  = wrapper_from_stack<editor_script::wrappers::ui::radio_button>(L, 2);
         luaL_argcheck(L, arg, 2, "radio_button expected");
         if (!self.button_group)
            return 0;
         if (!arg->widget)
            return 0;
         auto* group   = self.button_group;
         auto* button  = qobject_cast<QAbstractButton*>(arg->widget);
         auto* task    = new tasks::s2m::lambda(true);
         bool  success = false;
         task->handler = [group, button, &success]() {
            if (group->id(button) != -1) {
               success = true;
               button->setChecked(true);
            }
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         if (!success)
            luaL_error(L, "this radio group doesn't contain the given radio button");
         //
         return 0;
      }
      luastackchange_t selected_id(lua_State* L) {
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
         auto* group   = self.button_group;
         auto* task    = new tasks::s2m::lambda(true);
         bool  success = false;
         task->handler = [group, id, &success]() {
            auto* b = group->button(id);
            if (b) {
               success = true;
               b->setChecked(true);
            }
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         if (!success)
            luaL_error(L, "this radio group doesn't have a radio button with ID %d", id + 1); // increment to go from zero-indexed to Lua
         //
         return 0;
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.radio_group.new function should not be called with a colon or passed any arguments");
         //
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created]() {
            created = DovahKitScriptVMCore::get().try_spawn_button_group();
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         if (!created)
            return 0;
         wrapper out;
         auto* mt = wrap_button_group(out, *created);
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
      { "on",                    &_methods::on },
      { "remove",                &_methods::remove },
      { "remove_event_listener", &_methods::remove_event_listener },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "selected_button", &_getters::selected_button },
      { "selected_id",     &_getters::selected_id },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "selected_button", &_setters::selected_button },
      { "selected_id",     &_setters::selected_id },
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