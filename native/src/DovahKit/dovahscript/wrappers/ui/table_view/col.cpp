#include "col.h"
#include "../../../../helpers/lua/error.h"
#include "../../../../helpers/lua/qt_variant.h"
#include "../../../../helpers/qt/layout.h"
#include "../../../../ui/generic/ObservableStandardItemModel.h"
#include "../../../task_reference.h"
#include "../../../widget_overrides.h"
#include "../../../wrapper.h"
#include "../../../core/subsystems/coordinator.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"

#include "../../../tasks/s2m/ui_read_lambda.h"
#include "../../../tasks/s2m/ui_write_lambda.h"

#include "../../../api_helpers/model_observers.h"
#include "../../../api_helpers/model_observer_property_handlers.h"
#include "../../../api_helpers/widget_properties.h"
#include "../../../api_helpers/qt_alignment.h"
#include "../../../api_helpers/qt_color.h"

#include "col/collection_cells.h"

#include <QSortFilterProxyModel>

#include "../table_view.h"
#include "cell.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::ui::table_view_col;

   namespace _methods {
   }
   namespace _getters {
      int cells(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_table_view_span_cells;
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::ui::collections::table_view_col_cell_list.registry_key);
      }
      int index(lua_State* L) {
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
            core::subsystems::coordinator::get().send_ui_read_task(*task);
            delete task;
         }
         lua_pushinteger(L, result);
         return 1;
      }
      int text_color(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         QVariant result = api_helpers::get_model_items_data(self.model_observer, Qt::ForegroundRole);
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
         api_helpers::push_color(L, color);
         return 1;
      }
   }
   namespace _setters {
      int text_color(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         QColor color;
         //
         lua_settop(L, 2);
         switch (lua_type(L, 2)) {
            case LUA_TNONE:
            case LUA_TNIL:
               api_helpers::set_model_items_data(self.model_observer, Qt::ForegroundRole, QVariant());
               return 0;
            default:
               color = api_helpers::pull_color(L, 2);
               break;
         }
         api_helpers::set_model_items_data(self.model_observer, Qt::ForegroundRole, color);
         return 0;
      }
   }

   namespace _singleton_functions {
      int is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace dovahscript::wrappers::ui {
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

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      define_collection_metatable(L, wrappers::ui::collections::table_view_col_cell_list);
   }
   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}