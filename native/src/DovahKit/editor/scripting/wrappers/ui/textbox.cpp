#include "textbox.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"
#include "../../ui/util/alignment.h"

#include "helpers/widget_properties.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::textbox;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      luastackchange_t clear(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget]() { widget->clear(); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t redo(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget]() { widget->redo(); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t select_all(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget]() { widget->selectAll(); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t undo(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget]() { widget->undo(); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t alignment(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         std::string result;
         result.reserve(17); // "justify baseline"
         {
            auto* widget = (wrapped_type*)self.widget;
            auto* task   = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() {
               auto align = widget->alignment();
               editor_script::util::ui::alignment_to_string(align, result);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushstring(L, result.c_str());
         return 1;
      }
      luastackchange_t max_length(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::maxLength);
         lua_pushnumber(L, result);
         return 1;
      }
      luastackchange_t placeholder(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::placeholderText);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      luastackchange_t read_only(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::isReadOnly);
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::text);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      luastackchange_t validation_mask(lua_State* L) { // see QLineEdit::inputMask
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::inputMask);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      luastackchange_t value_visibility(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QLineEdit::EchoMode visibility = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QLineEdit::echoMode);
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
      luastackchange_t alignment(lua_State* L) {
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
            align = editor_script::util::ui::alignment_from_string(lua_tostring(L, 2), h, v, h_recognized, v_recognized);
            if (!h_recognized) {
               lua_warning(L, h.c_str(), 1);
               lua_warning(L, " is not a recognized horizontal alignment", 0);
            }
            if (!v_recognized) {
               lua_warning(L, v.c_str(), 1);
               lua_warning(L, " is not a recognized vertical alignment", 0);
            }
         }
         auto* widget  = (wrapped_type*)self.widget;
         auto* task    = new tasks::s2m::lambda(false);
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
            widget->setAlignment(align);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t max_length(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "max length (integer) expected");
         if (!self.widget)
            return 0;
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setMaxLength, lua_tointeger(L, 2));
         return 0;
      }
      luastackchange_t placeholder(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "placeholder (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setPlaceholderText, value);
         return 0;
      }
      luastackchange_t read_only(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setReadOnly, value);
         return 0;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setText, value);
         return 0;
      }
      luastackchange_t validation_mask(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "mask (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setInputMask, value);
         return 0;
      }
      luastackchange_t value_visibility(lua_State* L) {
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
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QLineEdit::setEchoMode, echo);
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
      { "clear",      &_methods::clear },
      { "redo",       &_methods::redo },
      { "select_all", &_methods::select_all },
      { "undo",       &_methods::undo },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "alignment",       &_getters::alignment },
      { "max_length",      &_getters::max_length },
      { "placeholder",     &_getters::placeholder },
      { "read_only",       &_getters::read_only },
      { "text",            &_getters::text },
      { "validation_mask", &_getters::validation_mask },
      { "value_visibility", &_getters::value_visibility },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "alignment",        &_setters::alignment },
      { "max_length",       &_setters::max_length },
      { "placeholder",      &_setters::placeholder },
      { "read_only",        &_setters::read_only },
      { "text",             &_setters::text },
      { "validation_mask",  &_setters::validation_mask },
      { "value_visibility", &_setters::value_visibility },
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