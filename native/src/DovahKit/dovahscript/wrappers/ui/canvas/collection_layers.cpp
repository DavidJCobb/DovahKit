#include "collection_layers.h"
#include "../../../../helpers/lua/error.h"
#include "../../../../helpers/qt/get_model_of.h"
#include "../../../../ui/generic/ObservableStandardItemModel.h"
#include "../../../core/subsystems/coordinator.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../tasks/s2m/ui_read_lambda.h"
#include "../../../push_native_object.h"
#include "../../../send_script_task.h"
#include "../../../task_reference.h"
#include "../../../wrapper.h"

#include "../canvas.h"
#include "layer.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.ui.canvas.layers>";
}

namespace {
   using namespace dovahscript;

   using widget_t = wrappers::ui::canvas::wrapped_type;
   
   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr)
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      if (self->widget == nullptr)
         cobb::lua::error(L, "function called with zombie self (expected %s)", collection_metatable_key);
      return *self;
   }
   
   int get_collection_length(lua_State* L) {
      auto& self  = get_collection_wrapper(L);
      int   result;
      {
         auto* task   = new tasks::s2m::ui_read_lambda;
         auto  widget = task_reference((widget_t*) self.widget);
         task->handler = [widget, &result]() {
            result = widget->layers().size();
         };
         send_script_ui_task(*task);
         delete task;
      }
      lua_pushinteger(L, result);
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      int isnum;
      int i = lua_tointegerx(L, 2, &isnum);
      if (!isnum)
         return 0;
      --i;
      if (i < 0)
         return 0;
      //
      CanvasWidgetEntity* layer = nullptr;
      {
         auto* task    = new tasks::s2m::ui_read_lambda;
         auto  widget  = task_reference((widget_t*) self.widget);
         task->handler = [widget, i, &layer]() {
            auto list = widget->layers();
            if (i >= list.size())
               return;
            layer = list[i];
         };
         send_script_ui_task(*task);
         delete task;
      }
      return push_native_object(layer);
   }
}

namespace dovahscript::wrappers::ui::collections {
   extern const collection_definition_params canvas_layers = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length = &get_collection_length,
      .lookup_item_by_index  = &lookup_item_by_index,
   };
}