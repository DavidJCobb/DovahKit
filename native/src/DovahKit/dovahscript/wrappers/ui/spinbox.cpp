#include "spinbox.h"
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
   using namespace dovahscript;
   using cls          = wrappers::ui::spinbox;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int step_by(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         int   steps = lua_tonumber(L, 2);
         if (!self.widget)
            return 0;
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, steps]() { widget->stepBy(steps); };
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
            auto* task   = new tasks::s2m::ui_read_lambda();
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
      int decimals(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::decimals);
         lua_pushinteger(L, result);
         return 1;
      }
      int maximum(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         double result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::maximum);
         lua_pushnumber(L, result);
         return 1;
      }
      int minimum(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         double result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::minimum);
         lua_pushnumber(L, result);
         return 1;
      }
      int prefix(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::prefix);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int read_only(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::isReadOnly);
         lua_pushboolean(L, result);
         return 1;
      }
      int step(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         double result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::singleStep);
         lua_pushnumber(L, result);
         return 1;
      }
      int suffix(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::suffix);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int value(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         double result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::value);
         lua_pushnumber(L, result);
         return 1;
      }
      int wraparound(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::wrapping);
         lua_pushboolean(L, result);
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
      int decimals(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "integer expected");
         if (!self.widget)
            return 0;
         auto value = lua_tointeger(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::setDecimals, value);
         return 0;
      }
      int maximum(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!self.widget)
            return 0;
         auto value = lua_tonumber(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::setMaximum, value);
         return 0;
      }
      int minimum(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!self.widget)
            return 0;
         auto value = lua_tonumber(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::setMinimum, value);
         return 0;
      }
      int prefix(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::setPrefix, value);
         return 0;
      }
      int read_only(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::setReadOnly, value);
         return 0;
      }
      int step(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!self.widget)
            return 0;
         auto value = lua_tonumber(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::setSingleStep, value);
         return 0;
      }
      int suffix(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::setSuffix, value);
         return 0;
      }
      int value(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!self.widget)
            return 0;
         auto value = lua_tonumber(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::setValue, value);
         return 0;
      }
      int wraparound(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QDoubleSpinBox::setWrapping, value);
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
            created->setKeyboardTracking(false); // only emit valueChanged (i.e. our Lua OnChanged) on Enter, blur, or arrow buttons
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
      { "step_by", &_methods::step_by },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "alignment",  &_getters::alignment },
      { "decimals",   &_getters::decimals },
      { "maximum",    &_getters::maximum },
      { "minimum",    &_getters::minimum },
      { "prefix",     &_getters::prefix },
      { "read_only",  &_getters::read_only },
      { "step",       &_getters::step },
      { "suffix",     &_getters::suffix },
      { "value",      &_getters::value },
      { "wraparound", &_getters::wraparound },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "alignment",  &_setters::alignment },
      { "decimals",   &_setters::decimals },
      { "maximum",    &_setters::maximum },
      { "minimum",    &_setters::minimum },
      { "prefix",     &_setters::prefix },
      { "read_only",  &_setters::read_only },
      { "step",       &_setters::step },
      { "suffix",     &_setters::suffix },
      { "value",      &_setters::value },
      { "wraparound", &_setters::wraparound },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}