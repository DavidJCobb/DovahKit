#include "text.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"
#include "../../cross_thread_tasks/s2m/spawn_window.h"

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
         if (!self.stub)
            return 0;
         std::string result;
         result.reserve(17); // "justify baseline"
         {
            auto* widget = (wrapped_type*)self.widget;
            auto* task   = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() {
               auto align = widget->alignment();
               if (align & Qt::AlignJustify)
                  result = "justify";
               else if (align & Qt::AlignHCenter)
                  result = "center";
               else if (align & Qt::AlignAbsolute) {
                  if (align & Qt::AlignLeft)
                     result = "left";
                  else if (align & Qt::AlignRight)
                     result = "right";
               } else {
                  if (align & Qt::AlignLeft)
                     result = "start";
                  else if (align & Qt::AlignRight)
                     result = "end";
               }
               if (result.empty())
                  result = "unchanged";
               //
               result += ' ';
               //
               if (align & Qt::AlignTop)
                  result += "top";
               else if (align & Qt::AlignBottom)
                  result += "bottom";
               else if (align & Qt::AlignVCenter)
                  result += "center";
               else if (align & Qt::AlignBaseline)
                  result += "baseline";
               else
                  result += "unchanged";
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushstring(L, result.c_str());
         return 1;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         QString result;
         {
            auto* widget = (wrapped_type*)self.widget;
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = widget->text(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      luastackchange_t word_wrap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         bool result;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = widget->wordWrap(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
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
            {
               auto*  value  = lua_tostring(L, 2);
               bool   second = false;
               size_t i = 0;
               char   c = value[i];
               do {
                  if (!c)
                     break;
                  if (c == ' ') {
                     if (second)
                        break;
                     if (!h.empty())
                        second = true;
                     continue;
                  }
                  c = tolower(c);
                  if (second)
                     v += c;
                  else
                     h += c;
               } while (c = value[++i]);
            }
            if (h == "left")
               align |= Qt::AlignLeft | Qt::AlignAbsolute;
            else if (h == "right")
               align |= Qt::AlignRight | Qt::AlignAbsolute;
            else if (h == "start")
               align |= Qt::AlignLeft;
            else if (h == "end")
               align |= Qt::AlignRight;
            else if (h == "justify")
               align |= Qt::AlignJustify;
            else if (h == "center")
               align |= Qt::AlignHCenter;
            else if (!h.empty() && h != "unchanged") {
               lua_warning(L, h.c_str(), 1);
               lua_warning(L, " is not a recognized horizontal alignment", 0);
            }
            //
            if (v == "top")
               align |= Qt::AlignTop;
            else if (v == "bottom")
               align |= Qt::AlignBottom;
            else if (v == "baseline")
               align |= Qt::AlignBaseline;
            else if (v == "center")
               align |= Qt::AlignVCenter;
            else if (!v.empty() && v != "unchanged") {
               lua_warning(L, v.c_str(), 1);
               lua_warning(L, " is not a recognized vertical alignment", 0);
            }
         }
         auto* widget = (wrapped_type*)self.widget;
         auto* task  = new tasks::s2m::lambda(false);
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
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         auto  value   = QString::fromUtf8(lua_tostring(L, 2));
         task->handler = [widget, value]() { widget->setText(value); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t word_wrap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.stub)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         bool  value   = lua_toboolean(L, 2);
         task->handler = [widget, value]() { widget->setWordWrap(value); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
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
            DovahKitScriptVM::get().accept_new_orphaned_widget(created);
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
      { "alignment", &_getters::alignment },
      { "text",      &_getters::text },
      { "word_wrap", &_getters::word_wrap },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "alignment", &_setters::alignment },
      { "text",      &_setters::text },
      { "word_wrap", &_setters::word_wrap },
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