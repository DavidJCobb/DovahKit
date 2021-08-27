#include "collection_tabs.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/coordinator.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../tasks/s2m/ui_read_lambda.h"
#include "../../../push_native_object.h"
#include "../../../send_script_task.h"
#include "../../../task_reference.h"
#include "../../../wrapper.h"

#include "../tabbox.h"
#include "tab.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.ui.tabbox.tabs>";
}

namespace {
   using namespace dovahscript;
   using widget_type = wrappers::ui::tabbox::wrapped_type;
   
   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr)
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      if (self->widget == nullptr)
         cobb::lua::error(L, "function called with zombie self (expected %s)", collection_metatable_key);
      return *self;
   }
   
   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      int   result;
      {
         auto* task    = new tasks::s2m::ui_read_lambda;
         auto  widget  = task_reference((widget_type*)self.widget);
         task->handler = [widget, &result]() {
            result = widget->count();
         };
         send_script_ui_task(*task);
         delete task;
      }
      lua_pushinteger(L, result);
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto  i    = lua_tointeger(L, 2) - 1; // lua indices start from one, not zero
      QWidget* result = nullptr;
      {
         auto* task    = new tasks::s2m::ui_read_lambda;
         auto  widget  = task_reference((widget_type*)self.widget);
         task->handler = [widget, i, &result]() {
            result = widget->widget(i);
         };
         send_script_ui_task(*task);
         delete task;
      }
      return push_native_object(result);
   }
}

namespace dovahscript::wrappers::ui::collections {
   extern const collection_definition_params tabbox_tabs = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length = &get_collection_length,
      .lookup_item_by_index  = &lookup_item_by_index,
   };
}