#include "line.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"

namespace {
   static constexpr std::array<std::pair<QFrame::Shape, const char*>, 4> _shapes = {{
      { QFrame::Shape::HLine, "horizontal" },
      { QFrame::Shape::VLine, "vertical" },
      { QFrame::Shape::HLine, "h" },
      { QFrame::Shape::VLine, "v" },
   }};
}

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::line;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t direction(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto* widget  = (wrapped_type*) self.widget;
            task->handler = [widget, &result]() {
               auto shape = widget->frameShape();
               for (const auto& p : _shapes) {
                  if (shape == p.first) {
                     result = p.second;
                     break;
                  }
               }
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t direction(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         QFrame::Shape shape = QFrame::Shape::NoFrame;
         {
            const char* arg = lua_tostring(L, 2);
            for (const auto& p : _shapes) {
               if (_stricmp(arg, p.second) == 0) {
                  shape = p.first;
                  break;
               }
            }
            if (shape == QFrame::Shape::NoFrame)
               luaL_error(L, "`%s` is not a recognized line direction", arg);
         }
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         auto  value   = QString::fromUtf8(lua_tostring(L, 2));
         task->handler = [widget, shape]() {
            widget->setFrameShape(shape);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         auto shape = QFrame::Shape::HLine;
         if (lua_gettop(L) > 0) {
            luaL_argcheck(L, lua_isstring(L, 1), 1, "nil or string expected");
            auto* arg   = lua_tostring(L, 1);
            bool  found = false;
            for (const auto& p : _shapes) {
               if (_stricmp(arg, p.second) == 0) {
                  shape = p.first;
                  found = true;
                  break;
               }
            }
            if (!found)
               luaL_error(L, "`%s` is not a recognized line direction", arg);
         }
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created, shape]() {
            created = new wrapped_type();
            created->setFrameShape(shape);
            created->setFrameShadow(QFrame::Sunken);
            override_widget_metatable(created, cls::metatable_key);
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
      { "direction", &_getters::direction },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "direction", &_setters::direction },
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