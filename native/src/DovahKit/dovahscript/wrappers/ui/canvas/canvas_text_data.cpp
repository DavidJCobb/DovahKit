#include "canvas_text_data.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/lifetime.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/collections.h"
#include "../../../push_native_object.h"
#include "../../../send_script_task.h"
#include "../../../task_reference.h"

#include "../../../tasks/s2m/ui_read_lambda.h"
#include "../../../tasks/s2m/ui_write_lambda.h"

#include "../../../api_helpers/qt_color.h"

#include "../../../qt/DovahscriptCanvasWidgetLayerDataText.h"
#include "../various/font.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::canvas_text_data;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      int color(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* cast = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!cast)
            return 0;
         QColor result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto  data    = task_reference(cast);
            task->handler = [data, &result]() {
               result = data->color;
            };
            send_script_ui_task(*task);
            delete task;
         }
         api_helpers::push_color(L, result);
         return 1;
      }
      int font(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* cast = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!cast)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::ui_font_role);
         return core::subsystems::userdata::get().push(L, out, wrappers::ui::font::metatable_key);
      }
      int max_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* cast = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!cast)
            return 0;
         lua_Number result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto  data    = task_reference(cast);
            task->handler = [data, &result]() {
               result = data->constrain.height();
            };
            send_script_ui_task(*task);
            delete task;
         }
         if (result <= 0.0)
            return 0;
         lua_pushnumber(L, result);
         return 1;
      }
      int max_width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* cast = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!cast)
            return 0;
         lua_Number result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto  data    = task_reference(cast);
            task->handler = [data, &result]() {
               result = data->constrain.width();
            };
            send_script_ui_task(*task);
            delete task;
         }
         if (result <= 0.0)
            return 0;
         lua_pushnumber(L, result);
         return 1;
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* cast = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!cast)
            return 0;
         QString result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto  data    = task_reference(cast);
            task->handler = [data, &result]() {
               result = data->text;
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int word_wrap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* cast = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         if (!cast)
            return 0;
         bool result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto  data    = task_reference(cast);
            task->handler = [data, &result]() {
               result = data->wordWrap;
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
   }
   namespace _setters {
      int color(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* cast  = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         auto  value = api_helpers::pull_color(L, 2);
         if (!cast)
            return 0;
         auto* task    = new tasks::s2m::ui_write_lambda;
         auto  data    = task_reference(cast);
         task->handler = [data, value]() {
            data->color = value;
            data->update();
         };
         send_script_ui_task(*task);
         return 0;
      }
      int font(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* cast = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         QFont font = wrappers::ui::font::pull(L, 2);
         if (!cast)
            return 0;
         auto  data    = task_reference(cast);
         auto* task    = new tasks::s2m::ui_write_lambda;
         task->handler = [data, font]() {
            data->font = font;
            data->update();
         };
         send_script_ui_task(*task);
         return 0;
      }
      int max_height(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* cast  = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         lua_Number value;
         if (lua_isnoneornil(L, 2))
            value = -1.0F;
         else {
            luaL_argcheck(L, lua_isnumber(L, 2), 2, "number or nil expected");
            value = lua_tonumber(L, 2);
         }
         if (!cast)
            return 0;
         auto* task    = new tasks::s2m::ui_write_lambda;
         auto  data    = task_reference(cast);
         task->handler = [data, value]() {
            data->constrain.setHeight(value);
            data->update();
         };
         send_script_ui_task(*task);
         return 0;
      }
      int max_width(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* cast  = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         lua_Number value;
         if (lua_isnoneornil(L, 2))
            value = -1.0F;
         else {
            luaL_argcheck(L, lua_isnumber(L, 2), 2, "number or nil expected");
            value = lua_tonumber(L, 2);
         }
         if (!cast)
            return 0;
         auto* task    = new tasks::s2m::ui_write_lambda;
         auto  data    = task_reference(cast);
         task->handler = [data, value]() {
            data->constrain.setWidth(value);
            data->update();
         };
         send_script_ui_task(*task);
         return 0;
      }
      int text(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* cast  = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto  value = QString::fromUtf8(lua_tostring(L, 2));
         if (!cast)
            return 0;
         auto* task    = new tasks::s2m::ui_write_lambda;
         auto  data    = task_reference(cast);
         task->handler = [data, value]() {
            data->text = value;
            data->update();
         };
         send_script_ui_task(*task);
         return 0;
      }
      int word_wrap(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* cast  = qobject_cast<wrapped_type*>(self.canvas_layer_data);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         bool  value = lua_toboolean(L, 2);
         if (!cast)
            return 0;
         auto* task    = new tasks::s2m::ui_write_lambda;
         auto  data    = task_reference(cast);
         task->handler = [data, value]() {
            data->wordWrap = value;
            data->update();
         };
         send_script_ui_task(*task);
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the ui.%s.new function should not be called with a colon or passed any arguments", cls::global_name);
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::ui_write_lambda(true);
         task->handler = [&created]() {
            created = new wrapped_type();
            core::subsystems::lifetime::get().on_non_hierarchy_object_created(*created);
         };
         send_script_ui_task(*task);
         delete task;
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
      { "color",      &_getters::color },
      { "font",       &_getters::font },
      { "max_height", &_getters::max_height },
      { "max_width",  &_getters::max_width },
      { "text",       &_getters::text },
      { "word_wrap",  &_getters::word_wrap },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "color",      &_setters::color },
      { "font",       &_setters::font },
      { "max_height", &_setters::max_height },
      { "max_width",  &_setters::max_width },
      { "text",       &_setters::text },
      { "word_wrap",  &_setters::word_wrap },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}