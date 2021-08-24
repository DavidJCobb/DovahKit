#include "textbox.h"
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
   using cls          = wrappers::ui::textbox;
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
      int max_length(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::maxLength);
         lua_pushnumber(L, result);
         return 1;
      }
      int placeholder(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::placeholderText);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int read_only(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::isReadOnly);
         lua_pushboolean(L, result);
         return 1;
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::text);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int validation_mask(lua_State* L) { // see QLineEdit::inputMask
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::inputMask);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int value_visibility(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QLineEdit::EchoMode visibility = api_helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::echoMode);
         switch (visibility) {
            case QLineEdit::EchoMode::Normal:
               lua_pushstring(L, "normal");
               break;
            case QLineEdit::EchoMode::NoEcho:
               lua_pushstring(L, "invisible");
               break;
            case QLineEdit::EchoMode::Password:
               lua_pushstring(L, "password");
               break;
            case QLineEdit::EchoMode::PasswordEchoOnEdit:
               lua_pushstring(L, "password char-by-char");
               break;
            default:
               lua_pushstring(L, "unknown");
               break;
         }
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
         auto* widget  = (wrapped_type*)self.widget;
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
      int max_length(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "max length (integer) expected");
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setMaxLength, lua_tointeger(L, 2));
         return 0;
      }
      int placeholder(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "placeholder (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setPlaceholderText, value);
         return 0;
      }
      int read_only(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setReadOnly, value);
         return 0;
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setText, value);
         return 0;
      }
      int validation_mask(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         QString value;
         if (!lua_isnoneornil(L, 2)) {
            luaL_argcheck(L, lua_isstring(L, 2), 2, "mask (string) expected");
            value = QString::fromUtf8(lua_tostring(L, 2));
         }
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setInputMask, value);
         return 0;
      }
      int value_visibility(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "visibility type (string) expected");
         if (!self.widget)
            return 0;
         QLineEdit::EchoMode echo = QLineEdit::EchoMode::Normal;
         {
            auto* vis = lua_tostring(L, 2);
            if (_stricmp(vis, "normal") == 0)
               echo = QLineEdit::EchoMode::Normal;
            else if (_stricmp(vis, "invisible") == 0)
               echo = QLineEdit::EchoMode::NoEcho;
            else if (_stricmp(vis, "password") == 0)
               echo = QLineEdit::EchoMode::Password;
            else if (_stricmp(vis, "password char-by-char") == 0)
               echo = QLineEdit::EchoMode::PasswordEchoOnEdit; // broken within Qt? Password works just fine, but this doesn't seem to do anything
            else
               luaL_error(L, "string \"%s\" is not a recognized visibility type", vis);
         }
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setEchoMode, echo);
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
      { "clear",      &_methods::clear },
      { "redo",       &_methods::redo },
      { "select_all", &_methods::select_all },
      { "undo",       &_methods::undo },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "alignment",       &_getters::alignment },
      { "max_length",      &_getters::max_length },
      { "placeholder",     &_getters::placeholder },
      { "read_only",       &_getters::read_only },
      { "text",            &_getters::text },
      { "validation_mask", &_getters::validation_mask },
      { "value_visibility", &_getters::value_visibility },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "alignment",        &_setters::alignment },
      { "max_length",       &_setters::max_length },
      { "placeholder",      &_setters::placeholder },
      { "read_only",        &_setters::read_only },
      { "text",             &_setters::text },
      { "validation_mask",  &_setters::validation_mask },
      { "value_visibility", &_setters::value_visibility },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}