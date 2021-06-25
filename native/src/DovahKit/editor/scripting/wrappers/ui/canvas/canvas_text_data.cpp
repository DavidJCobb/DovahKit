#include "canvas_text_data.h"
#include "../../../systems/editor_script_inner_core.h"
#include "../../../systems/messaging.h"
#include "../../../systems/permissions.h"
#include "../../../systems/userdata.h"

#include "../../../wrapper_util.h"
#include "../../../collections.h"

#include "../../../cross_thread_tasks/s2m/lambda.h"

#include "../../../ui/util/color.h"

#include "../various/font.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::canvas_text_data;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t color(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!data)
            return 0;
         QColor result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [data, &result]() {
               result = data->color;
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         util::ui::push_color(L, result);
         return 1;
      }
      luastackchange_t font(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!data)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::ui_font_role);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::ui::font::metatable_key);
      }
      luastackchange_t max_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!data)
            return 0;
         lua_Number result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [data, &result]() {
               result = data->constrain.height();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (result <= 0.0)
            return 0;
         lua_pushnumber(L, result);
         return 1;
      }
      luastackchange_t max_width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!data)
            return 0;
         lua_Number result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [data, &result]() {
               result = data->constrain.width();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (result <= 0.0)
            return 0;
         lua_pushnumber(L, result);
         return 1;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!data)
            return 0;
         QString result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [data, &result]() {
               result = data->text;
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      luastackchange_t word_wrap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!data)
            return 0;
         bool result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [data, &result]() {
               result = data->wordWrap;
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t color(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* data  = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         auto  value = util::ui::pull_color(L, 2);
         if (!data)
            return 0;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [data, value]() {
            data->color = value;
            data->update();
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t font(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* data  = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!data)
            return 0;
         QFont font    = wrappers::ui::font::pull(L, 2);
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [data, font]() {
            data->font = font;
            data->update();
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t max_height(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* data  = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         lua_Number value;
         if (lua_isnoneornil(L, 2))
            value = -1.0F;
         else {
            luaL_argcheck(L, lua_isnumber(L, 2), 2, "number or nil expected");
            value = lua_tonumber(L, 2);
         }
         if (!data)
            return 0;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [data, value]() {
            data->constrain.setHeight(value);
            data->update();
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t max_width(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* data  = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         lua_Number value;
         if (lua_isnoneornil(L, 2))
            value = -1.0F;
         else {
            luaL_argcheck(L, lua_isnumber(L, 2), 2, "number or nil expected");
            value = lua_tonumber(L, 2);
         }
         if (!data)
            return 0;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [data, value]() {
            data->constrain.setWidth(value);
            data->update();
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t text(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* data  = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto  value = QString::fromUtf8(lua_tostring(L, 2));
         if (!data)
            return 0;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [data, value]() {
            data->text = value;
            data->update();
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t word_wrap(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* data  = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         bool  value = lua_toboolean(L, 2);
         if (!data)
            return 0;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [data, value]() {
            data->wordWrap = value;
            data->update();
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.spinbox.new function should not be called with a colon or passed any arguments");
         //
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created]() {
            created = new wrapped_type();
            DovahKitScriptVMCore::get().set_up_new_canvas_layer_data(created);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         wrapper out;
         out.type = wrapper_type::ui_canvas_layer_data;
         out.canvas_layer_data = created;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::ui::canvas_text_data::metatable_key);
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
      { "color",      &_getters::color },
      { "font",       &_getters::font },
      { "max_height", &_getters::max_height },
      { "max_width",  &_getters::max_width },
      { "text",       &_getters::text },
      { "word_wrap",  &_getters::word_wrap },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "color",      &_setters::color },
      { "font",       &_setters::font },
      { "max_height", &_setters::max_height },
      { "max_width",  &_setters::max_width },
      { "text",       &_setters::text },
      { "word_wrap",  &_setters::word_wrap },
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