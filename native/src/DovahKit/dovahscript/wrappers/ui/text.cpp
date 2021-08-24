#include "text.h"
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

#include "various/font.h"

namespace {
   Qt::TextInteractionFlags flags_for_allow_selection = Qt::TextInteractionFlag::TextSelectableByKeyboard | Qt::TextInteractionFlag::TextSelectableByMouse;
}

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::text;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int clear(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget]() { widget->clear(); };
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
            auto* widget = (wrapped_type*)self.widget;
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
      int allow_selection(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = (widget->textInteractionFlags() & flags_for_allow_selection) != 0; };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
      int font(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::ui_font_data);
         return core::subsystems::userdata::get().push(L, out, wrappers::ui::font::metatable_key);
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QLabel::text);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int word_wrap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QLabel::wordWrap);
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
         auto  widget  = task_reference((wrapped_type*)self.widget);
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
      int allow_selection(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         //
         Qt::TextInteractionFlags flags;
         if (lua_toboolean(L, 2))
            flags = flags_for_allow_selection;
         //
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QLabel::setTextInteractionFlags, flags);
         return 0;
      }
      int font(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QFont font    = wrappers::ui::font::pull(L, 2);
         auto  widget  = task_reference(self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, font]() {
            widget->setFont(font);
         };
         send_script_ui_task(*task);
         return 0;
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         bool  html    = core::subsystems::permissions::check_ui_html_permissions();
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         auto  value   = QString::fromUtf8(lua_tostring(L, 2));
         task->handler = [widget, value, html]() {
            widget->setTextFormat(html ? Qt::TextFormat::AutoText : Qt::TextFormat::PlainText);
            widget->setText(value);
         };
         send_script_ui_task(*task);
         return 0;
      }
      int word_wrap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QLabel::setWordWrap, value);
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
         bool  html = core::subsystems::permissions::check_ui_html_permissions();
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
         task->configure = [html, text](wrapped_type* created) {
            created->setTextFormat(html ? Qt::TextFormat::AutoText : Qt::TextFormat::PlainText);
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
      { "clear", &_methods::clear },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "alignment",       &_getters::alignment },
      { "allow_selection", &_getters::allow_selection },
      { "font",            &_getters::font },
      { "text",            &_getters::text },
      { "word_wrap",       &_getters::word_wrap },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "alignment",       &_setters::alignment },
      { "allow_selection", &_setters::allow_selection },
      { "font",            &_setters::font },
      { "text",            &_setters::text },
      { "word_wrap",       &_setters::word_wrap },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}