#include "text.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"
#include "../../ui/util/alignment.h"

#include "various/font.h"

#include "helpers/widget_properties.h"

#include "../../../helpers/lua/warning.h"

namespace {
   Qt::TextInteractionFlags flags_for_allow_selection = Qt::TextInteractionFlag::TextSelectableByKeyboard | Qt::TextInteractionFlag::TextSelectableByMouse;
}

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::text;
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
      luastackchange_t allow_selection(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = (widget->textInteractionFlags() & flags_for_allow_selection) != 0; };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t font(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::ui_font_role);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::ui::font::metatable_key);
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QLabel::text);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      luastackchange_t word_wrap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QLabel::wordWrap);
         lua_pushboolean(L, result);
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
               cobb::lua::warning(L, "%s is not a recognized horizontal alignment", h.c_str());
            }
            if (!v_recognized) {
               cobb::lua::warning(L, "%s is not a recognized vertical alignment", v.c_str());
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
      luastackchange_t allow_selection(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         //
         Qt::TextInteractionFlags flags;
         if (lua_toboolean(L, 2))
            flags = flags_for_allow_selection;
         //
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QLabel::setTextInteractionFlags, flags);
         return 0;
      }
      luastackchange_t font(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QFont font    = wrappers::ui::font::pull(L, 2);
         auto* widget  = self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget, font]() {
            widget->setFont(font);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         bool  html    = DovahKitScriptVMPermissionInterface::check_ui_html_permissions();
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         auto  value   = QString::fromUtf8(lua_tostring(L, 2));
         task->handler = [widget, value, html]() {
            widget->setTextFormat(html ? Qt::TextFormat::AutoText : Qt::TextFormat::PlainText);
            widget->setText(value);
         };
         task->collapse_key = cobb::eight_cc("UITextTx");
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t word_wrap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QLabel::setWordWrap, value);
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
      { "clear", &_methods::clear },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "alignment",       &_getters::alignment },
      { "allow_selection", &_getters::allow_selection },
      { "font",            &_getters::font },
      { "text",            &_getters::text },
      { "word_wrap",       &_getters::word_wrap },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "alignment",       &_setters::alignment },
      { "allow_selection", &_setters::allow_selection },
      { "font",            &_setters::font },
      { "text",            &_setters::text },
      { "word_wrap",       &_setters::word_wrap },
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