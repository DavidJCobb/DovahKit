#include "collection_rows.h"
#include "../../../../helpers/lua/error.h"
#include "../../../../helpers/qt/get_model_of.h"
#include "../../../../ui/generic/ObservableStandardItemModel.h"
#include "../../../core/subsystems/coordinator.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../tasks/s2m/ui_read_lambda.h"
#include "../../../send_script_task.h"
#include "../../../task_reference.h"
#include "../../../wrapper.h"

#include "../table_view.h"
#include "row.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.ui.table_view.rows>";
}

namespace {
   using namespace dovahscript;

   using widget_t   = wrappers::ui::table_view::wrapped_type;
   using model_t    = ObservableStandardItemModel;
   using observer_t = ObservableStandardItemModelObserver;
   
   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr)
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      if (self->widget == nullptr)
         cobb::lua::error(L, "function called with zombie self (expected %s)", collection_metatable_key);
      return *self;
   }
   model_t& get_model(QWidget* widget) {
      auto* model = qobject_cast<model_t*>(cobb::qt::get_model_of(widget));
      assert(model);
      return *model;
   }
   model_t& get_model(const wrapper& w) {
      return get_model(w.widget);
   }

   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      int   count;
      {
         auto* task    = new tasks::s2m::ui_read_lambda;
         task->handler = [&self, &count]() {
            auto& model = get_model(self);
            count = model.rowCount();
         };
         send_script_ui_task(*task);
         delete task;
      }
      lua_pushinteger(L, count);
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self  = get_collection_wrapper(L);
      auto& model = get_model(self);
      auto  index = lua_tointeger(L, 2) - 1; // lua indices start from one, not zero
      if (index < 0)
         return 0;
      //
      observer_t* observer = nullptr;
      {
         auto* task    = new tasks::s2m::ui_read_lambda;
         task->handler = [&self, index, &observer]() {
            auto& model = get_model(self);
            if (index < 0 || index >= model.rowCount())
               return;
            observer = model.getOrCreateRegisteredObserver(QModelIndex(), model_t::rowOrientation, index);
         };
         send_script_ui_task(*task);
         delete task;
      }
      if (!observer)
         return 0;
      //
      wrapper iw;
      iw.type = wrapper_type::model_observer;
      iw.model_observer = observer;
      return core::subsystems::userdata::get().push(L, iw, wrappers::ui::table_view_row::metatable_key);
   }
}

namespace dovahscript::wrappers::ui::collections {
   extern const collection_definition_params table_view_row_list = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length = &get_collection_length,
      .lookup_item_by_index  = &lookup_item_by_index,
   };
}