#include "progress_bar.h"
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
#include "../../tasks/s2m/ui_write_lambda_ex.h"

#include "../../api_helpers/qt_alignment.h"
#include "../../api_helpers/widget_properties.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::progress_bar;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int reset(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* task = new tasks::s2m::ui_write_lambda_ex(false, [widget = task_reference((wrapped_type*)self.widget)]() {
            widget->reset();
         });
         send_script_ui_task(*task);
         return 0;
      }
   }
   namespace _getters {
      int alignment(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         std::string result;
         result.reserve(17); // "justify baseline"
         {
            auto  widget  = task_reference((wrapped_type*) self.widget);
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() {
               auto align = widget->alignment();
               api_helpers::alignment_to_string(align, result);
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushstring(L, result.c_str());
         return 1;
      }
      int current_text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::text);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int format(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::format);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int maximum(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::maximum);
         lua_pushinteger(L, result);
         return 1;
      }
      int minimum(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::minimum);
         lua_pushinteger(L, result);
         return 1;
      }
      int show_text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::isTextVisible);
         lua_pushboolean(L, result);
         return 1;
      }
      int value(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::value);
         lua_pushinteger(L, result);
         return 1;
      }
   }
   namespace _setters {
      int alignment(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         //
         Qt::Alignment align;
         {
            std::string h;
            std::string v;
            bool h_recognized;
            bool v_recognized;
            align = api_helpers::alignment_from_string(lua_tostring(L, 2), h, v, h_recognized, v_recognized);
            if (!h_recognized) {
               cobb::lua::warning(L, "%s is not a recognized horizontal alignment", h.c_str());
            }
            if (!v_recognized) {
               cobb::lua::warning(L, "%s is not a recognized vertical alignment", v.c_str());
            }
         }
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, align]() {
            Qt::Alignment after = align;
            Qt::Alignment prior = widget->alignment();
            auto ph = prior & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter | Qt::AlignJustify | Qt::AlignAbsolute);
            auto pv = prior & (Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter | Qt::AlignBaseline);
            auto ah = after & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter | Qt::AlignJustify | Qt::AlignAbsolute);
            auto av = after & (Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter | Qt::AlignBaseline);
            if (!ah)
               after |= ph;
            if (!av)
               after |= pv;
            widget->setAlignment(after);
         };
         send_script_ui_task(*task);
         return 0;
      }
      int format(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setFormat, value);
         return 0;
      }
      int maximum(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!self.widget)
            return 0;
         int value = lua_tointeger(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setMaximum, value);
         return 0;
      }
      int minimum(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!self.widget)
            return 0;
         int value = lua_tointeger(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setMinimum, value);
         return 0;
      }
      int show_text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         bool value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setTextVisible, value);
         return 0;
      }
      int value(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number expected (integer preferred)");
         int value = lua_tonumber(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setValue, value);
         return 0;

      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the ui.%s.new function should not be called with a colon or passed any arguments", cls::global_name);
         //
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
         task->configure = [](wrapped_type* created) {
            created->setAlignment(Qt::AlignmentFlag::AlignHCenter);
            created->setFormat("");
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
      { "reset", &_methods::reset },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "alignment",    &_getters::alignment },
      { "current_text", &_getters::current_text },
      { "format",       &_getters::format },
      { "maximum",      &_getters::maximum },
      { "minimum",      &_getters::minimum },
      { "show_text",    &_getters::show_text },
      { "value",        &_getters::value },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "alignment", &_setters::alignment },
      { "format",    &_setters::format },
      { "maximum",   &_setters::maximum },
      { "minimum",   &_setters::minimum },
      { "show_text", &_setters::show_text },
      { "value",     &_setters::value },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}