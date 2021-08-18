#include "collection_cells.h"
#include "../../../../../helpers/lua/error.h"
#include "../../../../../ui/generic/ObservableStandardItemModel.h"
#include "../../../../core/subsystems/coordinator.h"
#include "../../../../core/subsystems/permissions.h"
#include "../../../../core/subsystems/userdata.h"
#include "../../../../core/classes.h"
#include "../../../../tasks/s2m/ui_read_lambda.h"
#include "../../../../task_reference.h"
#include "../../../../wrapper.h"

#include "../cell.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.ui.table_view_row.cells>";
}

namespace {
   using namespace dovahscript;

   using model_t    = ObservableStandardItemModel;
   using observer_t = ObservableStandardItemModelObserver;
   
   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr)
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      if (self->model_observer == nullptr)
         cobb::lua::error(L, "function called with zombie self (expected %s)", collection_metatable_key);
      return *self;
   }
   model_t& get_model(const observer_t& o) {
      return *o.model;
   }

   int get_collection_length(lua_State* L) {
      auto& self   = get_collection_wrapper(L);
      int   result = 0;
      {
         auto  observer = task_reference(self.model_observer);
         auto* task     = new tasks::s2m::ui_read_lambda;
         task->handler = [observer, &result]() {
            result = get_model(*observer).columnCount();
         };
      }
      lua_pushinteger(L, result);
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self  = get_collection_wrapper(L);
      int   isnum;
      int   index = lua_tointegerx(L, 2, &isnum) - 1;
      if (!isnum)
         return 0;
      //
      observer_t* result = nullptr;
      {
         auto  observer = task_reference(self.model_observer);
         auto* task     = new tasks::s2m::ui_read_lambda;
         task->handler = [index, observer, &result]() {
            if (!observer->row)
               return;
            auto& model = get_model(*observer);
            auto* item  = model.item(observer->row, index);
            if (!item)
               return;
            result = model.getOrCreateRegisteredObserver(model.indexFromItem(item));
         };
         core::subsystems::coordinator::get().send_ui_read_task(*task);
         delete task;
      }
      //
      if (!result)
         return 0;
      wrapper iw;
      iw.type = wrapper_type::model_observer;
      iw.model_observer = result;
      return core::subsystems::userdata::get().push(L, iw, wrappers::ui::table_view_cell::metatable_key);
   }
}

namespace dovahscript::wrappers::ui::collections {
   extern const collection_definition_params table_view_row_cell_list = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length = &get_collection_length,
      .lookup_item_by_index  = &lookup_item_by_index,
   };
}