#include "color_button.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"
#include "../../wrapper.h"

#include "../../tasks/s2m/create_ui_widget.h"

#include "../../api_helpers/qt_color.h"
#include "../../api_helpers/widget_properties.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::color_button;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      int color(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QColor result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::color);
         api_helpers::push_color(L, result);
         return 1;
      }
   }
   namespace _setters {
      int color(lua_State* L) {
         auto&  self  = get_wrapper_for_thiscall<cls>(L);
         QColor value = api_helpers::pull_color(L, 2);
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property_and_block_signals((wrapped_type*)self.widget, &wrapped_type::setColor, value);
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         //
         QColor initial;
         if (lua_gettop(L) > 0) {
            std::string error;
            initial = api_helpers::protected_pull_color(L, 1, error);
            if (!error.empty())
               luaL_argerror(L, 1, error.c_str());
         }
         //
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
         task->configure = [initial](wrapped_type* created) {
            created->setColor(initial);
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
      { "color", &_getters::color },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "color", &_setters::color },
   };

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}