#include "line.h"
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

#include "../../api_helpers/widget_properties.h"

namespace {
   static constexpr std::array<std::pair<QFrame::Shape, const char*>, 4> _shapes = {{
      { QFrame::Shape::HLine, "horizontal" },
      { QFrame::Shape::VLine, "vertical" },
      { QFrame::Shape::HLine, "h" },
      { QFrame::Shape::VLine, "v" },
   }};
}

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::line;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      int direction(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto  widget  = task_reference((wrapped_type*) self.widget);
            task->handler = [widget, &result]() {
               auto shape = widget->frameShape();
               for (const auto& p : _shapes) {
                  if (shape == p.first) {
                     result = p.second;
                     break;
                  }
               }
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      int direction(lua_State* L) {
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
               cobb::lua::error(L, "`%s` is not a recognized line direction", arg);
         }
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         auto  value   = QString::fromUtf8(lua_tostring(L, 2));
         task->handler = [widget, shape]() {
            widget->setFrameShape(shape);
         };
         send_script_ui_task(*task);
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
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
               cobb::lua::error(L, "`%s` is not a recognized line direction", arg);
         }
         //
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
         task->configure = [shape](wrapped_type* created) {
            created->setFrameShape(shape);
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
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "direction", &_getters::direction },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "direction", &_setters::direction },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}