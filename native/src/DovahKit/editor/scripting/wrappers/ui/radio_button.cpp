#include "radio_button.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"

#include "helpers/widget_properties.h"

#include "radio_group.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::radio_button;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t checked(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QRadioButton::isChecked);
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t group(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         auto* result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QRadioButton::group);
         if (!result)
            return 0;
         wrapper out;
         auto* mt = wrap_button_group(out, *result);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t id(lua_State* L) {
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
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (result == -1)
            lua_pushnil(L);
         else
            lua_pushinteger(L, result + 1);
         return 1;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QRadioButton::text);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t checked(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QRadioButton::setChecked, value);
         return 0;
      }
      luastackchange_t group(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         QButtonGroup* group = nullptr;
         QButtonGroup* prior = nullptr;
         if (!lua_isnoneornil(L, 2)) {
            auto* arg = wrapper_from_stack<editor_script::wrappers::ui::radio_group>(L, 2);
            luaL_argcheck(L, arg, 2, "radio_group expected");
            group = arg->button_group;
         }
         if (!self.widget)
            return 0;
         auto* button  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(true);
         task->handler = [group, button, &prior]() {
            prior = button->group();
            if (prior == group)
               return;
            if (prior)
               prior->removeButton(button);
            if (group)
               group->addButton(button);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         if (prior != group) {
            if (prior)
               DovahKitScriptVMCore::get().button_group_lost_a_member(prior);
            if (group)
               DovahKitScriptVMCore::get().button_group_gained_a_member(group);
         }
         //
         return 0;
      }
      luastackchange_t id(lua_State* L) {
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
         auto* button   = (wrapped_type*) self.widget;
         auto* task     = new tasks::s2m::lambda(true);
         bool  in_group = false;
         bool  id_taken = false;
         task->handler  = [button, id, &in_group, &id_taken]() {
            auto* group = button->group();
            if (!group)
               return;
            in_group = true;
            auto* prior = group->button(id);
            if (prior && prior != button) {
               id_taken = true;
               return;
            }
            group->setId(button, id);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         if (!in_group)
            luaL_error(L, "you must assign the radio button to a radio group before you can set the button's ID");
         if (id_taken)
            luaL_error(L, "the ID %d is already taken by another radio button in this radio button's group", id + 1);
         //
         return 0;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QRadioButton::setText, value);
         return 0;
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         QString text;
         if (lua_gettop(L) > 0) {
            luaL_argcheck(L, lua_isstring(L, 1), 1, "nil or string expected");
            text = QString::fromUtf8(lua_tostring(L, 1));
         }
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created, &text]() {
            created = new wrapped_type(text);
            DovahKitScriptVMCore::get().set_up_new_scripted_widget(created);
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
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "checked", &_getters::checked },
      { "group",   &_getters::group },
      { "id",      &_getters::id },
      { "text",    &_getters::text },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "checked", &_setters::checked },
      { "group",   &_setters::group },
      { "id",      &_setters::id },
      { "text",    &_setters::text },
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