#include "scrollbox.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/warning.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"
#include "../../widget_overrides.h"
#include "../../wrapper.h"

#include "../../tasks/s2m/create_ui_widget.h"
#include "../../tasks/s2m/ui_read_lambda.h"
#include "../../tasks/s2m/ui_write_lambda.h"

#include "../../api_helpers/qt_alignment.h"
#include "../../api_helpers/widget_properties.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::scrollbox;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int scroll_to(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         //
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "x-coordinate (integer) expected");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum, 3, "y-coordinate (integer) expected");
         int w = lua_tointegerx(L, 4, &isnum);
         if (!isnum || w < 0)
            w = 0;
         int h = lua_tointegerx(L, 5, &isnum);
         if (!isnum || h < 0)
            h = 0;
         //
         {
            auto  widget  = task_reference((wrapped_type*) self.widget);
            auto* task    = new tasks::s2m::ui_write_lambda(true);
            task->handler = [widget, x, y, w, h]() {
               if (w || h)
                  widget->ensureVisible(x + w, y + h);
               widget->ensureVisible(x, y);
            };
            send_script_ui_task(*task);
            delete task;
         }
         return 0;
      }
   }
   namespace _getters {
      int body(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QWidget* result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QScrollArea::widget);
         return push_native_object(result);
      }
   }
   namespace _setters {
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the ui.%s.new function should not be called with a colon or passed any arguments", cls::global_name);
         //
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
         task->configure = [](wrapped_type* created) {
            auto* body = new QWidget(created);
            body->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            set_widget_forced_parent(body, created);
            created->setWidget(body);
            created->setWidgetResizable(true);
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
      { "scroll_to", &_methods::scroll_to },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "body", &_getters::body },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}