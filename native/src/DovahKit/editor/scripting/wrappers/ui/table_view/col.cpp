#include "col.h"
#include "../../../systems/editor_script_inner_core.h"
#include "../../../systems/messaging.h"
#include "../../../systems/permissions.h"
#include "../../../systems/userdata.h"

#include "../../../wrapper_util.h"
#include "../../../collections.h"

#include <QSortFilterProxyModel>
#include "../../../ui/util/color.h"
#include "../../../ui/util/lua_item_model.h"

#include "../../../cross_thread_tasks/s2m/lambda.h"
#include "../helpers/model_observer_data.h"

#include "cell.h"

#pragma region Collection: "cells"
namespace {
   using namespace editor_script;

   namespace _collections::cells {
      using cls        = wrappers::ui::table_view_col;
      using model_t    = ObservableStandardItemModel;
      using observer_t = ObservableStandardItemModelObserver;

      static constexpr auto collection_key = cls::cell_collection_key;

      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", collection_key);
         }
         if (self->model_observer == nullptr) {
            luaL_error(L, "function called with zombie self (expected %s)", collection_key);
         }
         return *self;
      }
      observer_t& get_observer(const wrapper& w) {
         return *w.model_observer;
      }
      model_t& get_model(const wrapper& w) {
         return *w.model_observer->model;
      }
      model_t& get_model(const observer_t& o) {
         return *o.model;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self  = get_collection_wrapper(L);
         auto& model = get_model(self);
         lua_pushinteger(L, model.rowCount());
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self  = get_collection_wrapper(L);
         auto& obs   = get_observer(self);
         auto& model = get_model(obs);
         //
         int isnum;
         int row = lua_tointegerx(L, 2, &isnum) - 1;
         int col = obs.col;
         assert(col >= 0 && "This isn't actually a column. What went wrong?");
         if (!isnum)
            return 0;
         //
         observer_t* observer = nullptr;
         {
            auto* task = new tasks::s2m::lambda(true);
            task->handler = [&self, row, col, &observer]() {
               auto& model = get_model(self);
               auto* item  = model.item(row, col);
               if (!item)
                  return;
               observer = model.getOrCreateRegisteredObserver(model.indexFromItem(item));
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (!observer)
            return 0;
         //
         wrapper iw;
         iw.type = wrapper_type::ui_model_item;
         iw.model_observer = observer;
         return DovahKitScriptVMUserdataInterface::get().push(L, iw, wrappers::ui::table_view_cell::metatable_key);
      }
   }
}
#pragma endregion

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::table_view_col;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t cells(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_table_view_span_cells;
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::cell_collection_key);
      }
      luastackchange_t index(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         int result;
         {
            auto* observer = self.model_observer;
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [observer, &result]() {
               result = observer->col;
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         return DovahKitScriptVMCore::get().push_to_lua(result);
      }
      luastackchange_t text_color(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         QVariant result = helpers::get_model_items_data(self.model_observer, Qt::ForegroundRole);
         QColor   color;
         switch (result.type()) {
            case QMetaType::QBrush:
               color = result.value<QBrush>().color();
               break;
            case QMetaType::QColor:
               color = result.value<QColor>();
               break;
            default:
               lua_pushnil(L);
               return 1;
         }
         lua_settop(L, 1);
         util::ui::push_color(L, color);
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t text_color(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         QColor color;
         //
         lua_settop(L, 2);
         switch (lua_type(L, 2)) {
            case LUA_TNONE:
            case LUA_TNIL:
               helpers::set_model_items_data(self.model_observer, Qt::ForegroundRole, QVariant());
               return 0;
            default:
               color = util::ui::pull_color(L, 2);
               break;
         }
         helpers::set_model_items_data(self.model_observer, Qt::ForegroundRole, color);
         return 0;
      }
   }

   namespace _singleton_functions {
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
      { "cells",      &_getters::cells },
      { "index",      &_getters::index },
      { "text_color", &_getters::text_color },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "text_color", &_setters::text_color },
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setfield(L, pos, cls::global_name);
      //
      // Set up collection:
      //
      editor_script::define_collection_metatable(L, {
         .registry_key          = cls::cell_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::cells::get_collection_length,
         .lookup_item_by_index   = &_collections::cells::lookup_item_by_index,
      });
   }
}