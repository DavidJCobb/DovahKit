#include "collection_items.h"
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

#include "../dropdown.h"
#include "item.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.ui.dropdown.items>";
}

namespace {
   using namespace dovahscript;

   using widget_t   = wrappers::ui::dropdown::wrapped_type;
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
      auto& self  = get_collection_wrapper(L);
      int   count = 0;
      {
         auto* task = new tasks::s2m::ui_read_lambda;
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
      auto  row   = lua_tointeger(L, 2) - 1; // lua indices start from one, not zero
      if (row < 0)
         return 0;
      //
      observer_t* observer = nullptr;
      {
         auto* task = new tasks::s2m::ui_read_lambda;
         task->handler = [&self, row, &observer]() {
            auto& model = get_model(self);
            auto* root  = model.invisibleRootItem();
            auto* item  = root->child(row);
            if (!item)
               return;
            observer = model.getOrCreateRegisteredObserver(model.indexFromItem(item));
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
      return core::subsystems::userdata::get().push(L, iw, wrappers::ui::dropdown_item::metatable_key);
   }
}

namespace dovahscript::wrappers::ui::collections {
   extern const collection_definition_params dropdown_items = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length = &get_collection_length,
      .lookup_item_by_index  = &lookup_item_by_index,
   };
}