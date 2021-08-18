#include "table_view.h"
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/qt_variant.h"
#include "../../../helpers/qt/layout.h"
#include "../../../ui/generic/ObservableStandardItemModel.h"
#include "../../push_native_object.h"
#include "../../task_reference.h"
#include "../../widget_overrides.h"
#include "../../wrapper.h"
#include "../../core/subsystems/coordinator.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/resources/DovahscriptResourceStyledItemDelegate.h"
#include "../../core/subsystems/userdata.h"

#include "../../tasks/s2m/create_ui_widget.h"
#include "../../tasks/s2m/lambda.h"
#include "../../tasks/s2m/ui_read_lambda.h"
#include "../../tasks/s2m/ui_write_lambda.h"

#include "../../api_helpers/model_observers.h"
#include "../../api_helpers/model_observer_property_handlers.h"
#include "../../api_helpers/widget_properties.h"
#include "../../constants/ui_count_limits.h"

#include "table_view/collection_columns.h"
#include "table_view/collection_rows.h"
#include "table_view/row.h"
#include "table_view/col.h"
#include "table_view/cell.h"

namespace {
   // If a table has more than this many rows, then word-wrapping will no longer be allowed. This is 
   // a safety measure to prevent scripts with large tables from compromising performance; wrapping 
   // in huge tables can and will hang the main thread.
   static constexpr int max_wrappable_word_count = 400;

   enum class _word_wrap_mode {
      none,
      truncate,
      wrap,
   };

   _word_wrap_mode get_word_wrapping_for(QTableView* widget) {
      if (!widget->wordWrap())
         return _word_wrap_mode::none;
      auto vh = widget->verticalHeader();
      if (!vh->count()) {
         auto data = widget->property("Lua word wrap");
         if (data.isValid()) {
            return data.toBool() ? _word_wrap_mode::wrap : _word_wrap_mode::truncate;
         }
         return _word_wrap_mode::truncate;
      }
      return vh->sectionResizeMode(0) == QHeaderView::ResizeMode::ResizeToContents ? _word_wrap_mode::wrap : _word_wrap_mode::truncate;
   }
   void set_word_wrapping_for(QTableView* widget, _word_wrap_mode wm) {
      widget->setWordWrap(wm != _word_wrap_mode::none);
      widget->setProperty("Lua word wrap", wm == _word_wrap_mode::wrap); // needed because we can't check the header view's global resize mode if it's empty (ugh)
      auto* vh = widget->verticalHeader();
      if (wm == _word_wrap_mode::wrap)
         vh->setSectionResizeMode(QHeaderView::ResizeToContents);
      else
         vh->setSectionResizeMode(QHeaderView::Interactive);
   }
   void disable_word_wrap_without_changing_truncation(QTableView* widget) {
      widget->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
      widget->setProperty("Lua word wrap", false);
   }

   void _extract_cell_arg_list(lua_State* L, int first, QVector<dovahscript::api_helpers::moph::handler_set::role_map_t>& roles) {
      using namespace dovahscript;
      using handler     = dovahscript::api_helpers::moph::model_observer_property_handler;
      using handler_set = dovahscript::api_helpers::moph::handler_set;

      auto& core = DovahKitScriptVMCore::get();
      int   argcount = lua_gettop(L);
      for (int i = first; i <= argcount; ++i) {
         auto type = lua_type(L, i);
         if (type == LUA_TTABLE) {
            auto e = wrappers::ui::table_view_cell::moph_handlers.extract(L, i);
            roles.push_back(e);
            continue;
         }
         handler_set::role_map_t e;
         e[Qt::DisplayRole] = core.variant_from_lua(i);
         roles.push_back(e);
      }
   }
}

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::table_view;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      using role_map_t = api_helpers::moph::handler_set::role_map_t;

      int append_column(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         //
         // This function is designed in mimicry of the append-row function.
         //
         QVector<role_map_t> roles;
         _extract_cell_arg_list(L, 2, roles);
         //
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, roles]() { // do NOT pass args by reference, as this lambda is set not to block, so it'll go out of scope if you do!
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            //
            int size = roles.size();
            QList<QStandardItem*> to_append;
            to_append.reserve(size);
            for (int i = 0; i < size; ++i) {
               auto& data = roles[i];
               auto* item = new QStandardItem();
               for (auto it = data.begin(); it != data.end(); ++it)
                  item->setData(it.value(), it.key());
               to_append.push_back(item);
            }
            model->appendColumn(to_append);
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         //
         return 0;
      }
      int append_column_with_options(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         cobb::lua::argcheck(L, lua_istable(L, 2) || lua_isuserdata(L, 2), 2, "options table or userdata expected");
         //
         role_map_t span_roles = wrappers::ui::table_view_cell::moph_handlers.extract(L, 2);
         QVector<role_map_t> roles;
         _extract_cell_arg_list(L, 3, roles);
         //
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, span_roles, roles]() { // do NOT pass args by reference, as this lambda is set not to block, so it'll go out of scope if you do!
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            //
            int size = roles.size();
            QList<QStandardItem*> to_append;
            to_append.reserve(size);
            for (int i = 0; i < size; ++i) {
               auto& data = roles[i];
               auto* item = new QStandardItem();
               for (auto it = data.begin(); it != data.end(); ++it)
                  item->setData(it.value(), it.key());
               to_append.push_back(item);
            }
            model->appendColumn(to_append);
            //
            int col = model->columnCount() - 1;
            for (auto it = span_roles.begin(); it != span_roles.end(); ++it)
               model->setDefaultDataForSpan(it.key(), model->colOrientation, col, it.value());
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         //
         return 0;
      }
      int append_row(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         //
         // This function's arguments are the cells to add to the new row. Each argument can take 
         // one of the following forms:
         // 
         //  - An already-existing table cell, possibly one which has been orphaned from the table 
         //    view that originally created it.
         // 
         //  - A Lua table containing key/value pairs, to be processed by a model observer property 
         //    handler set; this can be used to preconfigure any supported Qt::ItemDataRole on the 
         //    newly-created cell.
         // 
         //  - Any other value, which will be used for the Qt::DisplayRole and likely coerced to a 
         //    string in the process.
         //
         QVector<role_map_t> roles;
         _extract_cell_arg_list(L, 2, roles);
         //
         // We've extracted the Lua arguments. Now, let's pass them in.
         //
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, roles]() { // do NOT pass (args) by reference, as this lambda is set not to block, so it'll go out of scope if you do!
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            if (model->rowCount() >= max_wrappable_word_count) // safety measure for large tables
               disable_word_wrap_without_changing_truncation(widget);
            //
            int size = roles.size();
            QList<QStandardItem*> to_append;
            to_append.reserve(size);
            for (int i = 0; i < size; ++i) {
               auto& data = roles[i];
               auto* item = new QStandardItem();
               for (auto it = data.begin(); it != data.end(); ++it)
                  item->setData(it.value(), it.key());
               to_append.push_back(item);
            }
            model->appendRow(to_append);
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         //
         return 0;
      }
      int append_row_with_options(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         cobb::lua::argcheck(L, lua_istable(L, 2) || lua_isuserdata(L, 2), 2, "options table or userdata expected");
         //
         role_map_t span_roles = wrappers::ui::table_view_cell::moph_handlers.extract(L, 2);
         QVector<role_map_t> roles;
         _extract_cell_arg_list(L, 3, roles);
         //
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, span_roles, roles]() { // do NOT pass (args) by reference, as this lambda is set not to block, so it'll go out of scope if you do!
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            if (model->rowCount() >= max_wrappable_word_count) // safety measure for large tables
               disable_word_wrap_without_changing_truncation(widget);
            //
            int size = roles.size();
            QList<QStandardItem*> to_append;
            to_append.reserve(size);
            for (int i = 0; i < size; ++i) {
               auto& data = roles[i];
               auto* item = new QStandardItem();
               for (auto it = data.begin(); it != data.end(); ++it)
                  item->setData(it.value(), it.key());
               to_append.push_back(item);
            }
            model->appendRow(to_append);
            //
            int row = model->rowCount() - 1;
            for (auto it = span_roles.begin(); it != span_roles.end(); ++it)
               model->setDefaultDataForSpan(it.key(), model->rowOrientation, row, it.value());
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         //
         return 0;
      }
      int clear(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         api_helpers::remove_items_from_model(self.widget, -2, -2);
         return 0;
      }
      int insert_column(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int isnum;
         int insert_at = lua_tointegerx(L, 2, &isnum);
         cobb::lua::argcheck(L, isnum, 2, "integer expected");
         if (--insert_at < 0) {
            luaL_argerror(L, 2, "column numbers start at 1");
         }
         //
         QVector<role_map_t> roles;
         _extract_cell_arg_list(L, 3, roles);
         //
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::ui_write_lambda(true);
         task->handler = [widget, roles, insert_at]() mutable { // do NOT pass args by reference, as this lambda is set not to block, so it'll go out of scope if you do!
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            //
            int size = roles.size();
            QList<QStandardItem*> to_append;
            to_append.reserve(size);
            for (int i = 0; i < size; ++i) {
               auto& data = roles[i];
               auto* item = new QStandardItem();
               for (auto it = data.begin(); it != data.end(); ++it)
                  item->setData(it.value(), it.key());
               to_append.push_back(item);
            }
            int cc = model->columnCount();
            if (insert_at > cc)
               insert_at = cc;
            model->insertColumn(insert_at, to_append);
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         delete task;
         //
         return 0;
      }
      int insert_row(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int isnum;
         int insert_at = lua_tointegerx(L, 2, &isnum);
         cobb::lua::argcheck(L, isnum, 2, "integer expected");
         if (--insert_at < 0) {
            luaL_argerror(L, 2, "row numbers start at 1");
         }
         //
         QVector<role_map_t> roles;
         _extract_cell_arg_list(L, 3, roles);
         //
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::ui_write_lambda(true);
         task->handler = [widget, roles, insert_at]() mutable { // do NOT pass args by reference, as this lambda is set not to block, so it'll go out of scope if you do!
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            if (model->rowCount() >= max_wrappable_word_count) // safety measure for large tables
               disable_word_wrap_without_changing_truncation(widget);
            //
            int size = roles.size();
            QList<QStandardItem*> to_append;
            to_append.reserve(size);
            for (int i = 0; i < size; ++i) {
               auto& data = roles[i];
               auto* item = new QStandardItem();
               for (auto it = data.begin(); it != data.end(); ++it)
                  item->setData(it.value(), it.key());
               to_append.push_back(item);
            }
            int rc = model->rowCount();
            if (insert_at > rc)
               insert_at = rc;
            model->insertRow(insert_at, to_append);
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         delete task;
         //
         return 0;
      }
      int remove_column(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int   isnum;
         int   col  = lua_tointegerx(L, 2, &isnum);
         if (!isnum) {
            auto* wrapper = wrapper_from_stack<wrappers::ui::table_view_col>(L, 2);
            cobb::lua::argcheck(L, wrapper != nullptr, 2, "expected an integer or table_view_col");
            if (!wrapper->model_observer)
               return 0;
            col = wrapper->model_observer->col;
            if (col == -1)
               return 0;
         } else {
            cobb::lua::argcheck(L, col > 0, 2, "table view columns are numbered from 1");
         }
         if (!self.widget)
            return 0;
         --col;
         api_helpers::remove_items_from_model(self.widget, -2, col);
         return 0;
      }
      int remove_row(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int   isnum;
         int   row = lua_tointegerx(L, 2, &isnum);
         if (!isnum) {
            auto* wrapper = wrapper_from_stack<wrappers::ui::table_view_row>(L, 2);
            cobb::lua::argcheck(L, wrapper != nullptr, 2, "expected an integer or table_view_row");
            if (!wrapper->model_observer)
               return 0;
            row = wrapper->model_observer->row;
            if (row == -1)
               return 0;
         } else {
            cobb::lua::argcheck(L, row > 0, 2, "table view rows are numbered from 1");
         }
         if (!self.widget)
            return 0;
         --row;
         api_helpers::remove_items_from_model(self.widget, row, -2);
         return 0;
      }
   }
   namespace _getters {
      int alternate_row_colors(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::alternatingRowColors);
         lua_pushboolean(L, result);
         return 1;
      }
      int column_headers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QVector<QString> headers;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto* widget  = (wrapped_type*)self.widget;
            task->handler = [widget, &headers]() {
               auto* proxy = (QSortFilterProxyModel*) widget->model();
               auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
               int   cc    = model->columnCount();
               headers.reserve(cc);
               for (int i = 0; i < cc; ++i) {
                  auto data = model->headerData(i, Qt::Horizontal, Qt::DisplayRole);
                  headers.push_back(data.toString());
               }
            };
            core::subsystems::coordinator::get().send_ui_read_task(*task);
            delete task;
         }
         lua_createtable(L, headers.size(), 0);
         for (int i = 0; i < headers.size(); ++i) {
            lua_pushstring(L, headers[i].toUtf8());
            lua_rawseti(L, -2, i + 1);
         }
         return 1;
      }
      int columns(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_table_view_cols;
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, cls::col_collection_key);
      }
      int has_corner_button(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::isCornerButtonEnabled);
         lua_pushboolean(L, result);
         return 1;
      }
      int min_column_width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = -1;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto* widget  = (wrapped_type*)self.widget;
            task->handler = [widget, &result]() {
               auto* header = widget->horizontalHeader();
               if (!header)
                  return;
               result = header->minimumSectionSize();
            };
            core::subsystems::coordinator::get().send_ui_read_task(*task);
            delete task;
         }
         if (result < 0)
            lua_pushnil(L);
         else
            lua_pushinteger(L, result);
         return 1;
      }
      int rows(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_table_view_rows;
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, cls::row_collection_key);
      }
      int selection(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         std::vector<ObservableStandardItemModelObserver*> observers;
         const char* metatable_key = nullptr;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto* widget  = (wrapped_type*) self.widget;
            task->handler = [widget, &observers, &metatable_key]() {
               auto* proxy = (QSortFilterProxyModel*) widget->model();
               auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
               auto* sm    = widget->selectionModel();
               switch (widget->selectionBehavior()) {
                  case QAbstractItemView::SelectionBehavior::SelectRows:
                     metatable_key = dovahscript::wrappers::ui::table_view_row::metatable_key;
                     for (auto& qmi : sm->selectedRows()) {
                        auto  remapped = proxy->mapToSource(qmi);
                        auto* o        = model->getOrCreateRegisteredObserver(QModelIndex(), ObservableStandardItemModel::rowOrientation, remapped.row());
                        assert(o);
                        observers.push_back(o);
                     }
                     break;
                  case QAbstractItemView::SelectionBehavior::SelectColumns:
                     metatable_key = dovahscript::wrappers::ui::table_view_col::metatable_key;
                     for (auto& qmi : sm->selectedColumns()) {
                        auto  remapped = proxy->mapToSource(qmi);
                        auto* o        = model->getOrCreateRegisteredObserver(QModelIndex(), ObservableStandardItemModel::colOrientation, remapped.column());
                        assert(o);
                        observers.push_back(o);
                     }
                     break;
                  case QAbstractItemView::SelectionBehavior::SelectItems:
                     metatable_key = dovahscript::wrappers::ui::table_view_cell::metatable_key;
                     for (auto& qmi : sm->selectedIndexes()) {
                        auto  remapped = proxy->mapToSource(qmi);
                        auto* o        = model->getOrCreateRegisteredObserver(remapped);
                        assert(o);
                        observers.push_back(o);
                     }
                     break;
               }
            };
            core::subsystems::coordinator::get().send_ui_read_task(*task);
            delete task;
         }
         size_t size = observers.size();
         lua_createtable(L, size, 0);
         int tbl = lua_gettop(L);
         for (size_t i = 0; i < size; ++i) {
            wrapper iw;
            iw.type = wrapper_type::model_observer;
            iw.model_observer = observers[i];
            int count = core::subsystems::userdata::get().push(L, iw, metatable_key);
            if (count) {
               lua_rawseti(L, tbl, i + 1);
               if (count > 1)
                  lua_pop(L, count - 1);
            }
         }
         return 1;
      }
      int selection_mode(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         auto result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::selectionMode);
         switch (result) {
            case QAbstractItemView::SelectionMode::SingleSelection:
               lua_pushstring(L, "single");
               return 1;
            case QAbstractItemView::SelectionMode::MultiSelection:
               lua_pushstring(L, "toggle");
               return 1;
            case QAbstractItemView::SelectionMode::ExtendedSelection:
               lua_pushstring(L, "multiple");
               return 1;
            case QAbstractItemView::SelectionMode::ContiguousSelection:
               lua_pushstring(L, "contiguous");
               return 1;
            case QAbstractItemView::SelectionMode::NoSelection:
               lua_pushstring(L, "disabled");
               return 1;
         }
         lua_pushstring(L, "invalid");
         return 1;
      }
      int selection_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         auto result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::selectionBehavior);
         switch (result) {
            case QAbstractItemView::SelectionBehavior::SelectRows:
               lua_pushstring(L, "rows");
               return 1;
            case QAbstractItemView::SelectionBehavior::SelectColumns:
               lua_pushstring(L, "columns");
               return 1;
            case QAbstractItemView::SelectionBehavior::SelectItems:
               lua_pushstring(L, "cells");
               return 1;
         }
         lua_pushstring(L, "invalid");
         return 1;
      }
      int show_column_headers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result;
         {
            auto* task = new tasks::s2m::ui_read_lambda();
            auto* widget = (wrapped_type*)self.widget;
            task->handler = [widget, &result]() {
               result = !widget->horizontalHeader()->isHidden();
            };
            core::subsystems::coordinator::get().send_ui_read_task(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
      int show_grid(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::showGrid);
         lua_pushboolean(L, result);
         return 1;
      }
      int show_row_headers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto* widget  = (wrapped_type*)self.widget;
            task->handler = [widget, &result]() {
               result = !widget->verticalHeader()->isHidden();
            };
            core::subsystems::coordinator::get().send_ui_read_task(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
      int sortable(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::isSortingEnabled);
         lua_pushboolean(L, result);
         return 1;
      }
      int word_wrap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         _word_wrap_mode ww;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto* widget  = (wrapped_type*)self.widget;
            task->handler = [widget, &ww]() {
               ww = get_word_wrapping_for(widget);
            };
            core::subsystems::coordinator::get().send_ui_read_task(*task);
            delete task;
         }
         switch (ww) {
            case _word_wrap_mode::none:
               lua_pushstring(L, "none");
               return 1;
            case _word_wrap_mode::truncate:
               lua_pushstring(L, "truncate");
               return 1;
            case _word_wrap_mode::wrap:
               lua_pushstring(L, "wrap");
               return 1;
         }
         lua_pushstring(L, "invalid");
         return 1;
      }
   }
   namespace _setters {
      int alternate_row_colors(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setAlternatingRowColors, value);
         return 0;
      }
      int column_headers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int count = 0;
         lua_settop(L, 2);
         if (!lua_isnoneornil(L, 2)) {
            auto type = lua_type(L, 2);
            if (type != LUA_TUSERDATA && type != LUA_TTABLE)
               luaL_argerror(L, 2, "table or userdata expected");
            lua_len(L, 2);
            int isnum;
            count = lua_tointegerx(L, 3, &isnum);
            lua_pop(L, 1);
            cobb::lua::argcheck(L, isnum, 2, "argument's length operator did not return an integer");
         }
         QStringList text;
         text.reserve(count);
         for (int i = 1; i <= count; ++i) {
            lua_geti(L, 2, i);
            text.push_back(lua_tostring(L, 3));
            lua_pop(L, 1);
         }
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         auto* widget  = (wrapped_type*)self.widget;
         task->handler = [widget, text]() {
            auto* proxy = (QSortFilterProxyModel*)widget->model();
            auto* model = (ObservableStandardItemModel*)proxy->sourceModel();
            model->setHorizontalHeaderLabels(text);
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         return 0;
      }
      int has_corner_button(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setCornerButtonEnabled, value);
         return 0;
      }
      int min_column_width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int width = lua_tointegerx(L, 2, &isnum);
         if (isnum) {
            cobb::lua::argcheck(L, width >= 0, 2, "negative values are not valid here");
         } else {
            cobb::lua::argcheck(L, lua_isnoneornil(L, 2), 2, "integer or nil expected");
            width = -1;
         }
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         auto* widget  = (wrapped_type*)self.widget;
         task->handler = [widget, width]() {
            auto* header = widget->horizontalHeader();
            if (header)
               header->setMinimumSectionSize(width);
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         return 0;
      }
      int selection_mode(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         QAbstractItemView::SelectionMode value;
         {
            auto* arg = lua_tostring(L, 2);
            if (_stricmp(arg, "single") == 0)
               value = QAbstractItemView::SelectionMode::SingleSelection;
            else if (_stricmp(arg, "toggle") == 0)
               value = QAbstractItemView::SelectionMode::MultiSelection;
            else if (_stricmp(arg, "multiple") == 0)
               value = QAbstractItemView::SelectionMode::ExtendedSelection;
            else if (_stricmp(arg, "contiguous") == 0)
               value = QAbstractItemView::SelectionMode::ContiguousSelection;
            else if (_stricmp(arg, "disabled") == 0)
               value = QAbstractItemView::SelectionMode::NoSelection;
            else
               luaL_error(L, "string `%s` is not a recognized selection type", arg);
         }
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setSelectionMode, value);
         return 0;
      }
      int selection_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         QAbstractItemView::SelectionBehavior value;
         {
            auto* arg = lua_tostring(L, 2);
            if (_stricmp(arg, "rows") == 0)
               value = QAbstractItemView::SelectionBehavior::SelectRows;
            else if (_stricmp(arg, "columns") == 0)
               value = QAbstractItemView::SelectionBehavior::SelectColumns;
            else if (_stricmp(arg, "cells") == 0)
               value = QAbstractItemView::SelectionBehavior::SelectItems;
            else
               luaL_error(L, "string `%s` is not a recognized selection type", arg);
         }
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setSelectionBehavior, value);
         return 0;
      }
      int show_column_headers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto  value   = lua_toboolean(L, 2);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         auto* widget  = (wrapped_type*) self.widget;
         task->handler = [widget, value]() {
            widget->horizontalHeader()->setHidden(!value);
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         return 0;
      }
      int show_grid(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setShowGrid, value);
         return 0;
      }
      int show_row_headers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto  value   = lua_toboolean(L, 2);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         auto* widget  = (wrapped_type*) self.widget;
         task->handler = [widget, value]() {
            widget->verticalHeader()->setHidden(!value);
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         return 0;
      }
      int sortable(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setSortingEnabled, value);
         return 0;
      }
      int word_wrap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         _word_wrap_mode ww;
         {
            auto* vis = lua_tostring(L, 2);
            if (_stricmp(vis, "none") == 0)
               ww = _word_wrap_mode::none;
            else if (_stricmp(vis, "truncate") == 0)
               ww = _word_wrap_mode::truncate;
            else if (_stricmp(vis, "wrap") == 0)
               ww = _word_wrap_mode::wrap;
            else
               luaL_error(L, "string `%s` is not a recognized word wrap type", vis);
         }
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         auto* widget  = (wrapped_type*)self.widget;
         task->handler = [widget, ww]() {
            set_word_wrapping_for(widget, ww);
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the ui.widget.new function should not be called with a colon or passed any arguments");
         //
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
         //
         // If you want to configure the widget, e.g. to provide sensible defaults, you would 
         // do it in this optional handler.
         //
         task->configure = [](wrapped_type* created) {
            created->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
            created->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows); // sensible defaults
            created->setCornerButtonEnabled(false); // sensible defaults
            created->horizontalHeader()->setStretchLastSection(true);
            auto* vh = created->verticalHeader();
            vh->setDefaultSectionSize(0); // get rid of weird padding
            vh->setHidden(true); // sensible defaults
            //
            created->setItemDelegate(new DovahscriptResourceStyledItemDelegate(created));
         };
         //
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         auto* created = task->created;
         delete task;
         //
         if (!created) {
            cobb::lua::error(L, "the script is only allowed to create %d windows", max_script_windows);
         }
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
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "append_column",              &_methods::append_column },
      { "append_column_with_options", &_methods::append_column_with_options },
      { "append_row",                 &_methods::append_row },
      { "append_row_with_options",    &_methods::append_row_with_options },
      { "clear",                      &_methods::clear },
      { "insert_column",              &_methods::insert_column },
      { "insert_row",                 &_methods::insert_row },
      { "remove_column",              &_methods::remove_column },
      { "remove_row",                 &_methods::remove_row },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "alternate_row_colors", &_getters::alternate_row_colors },
      { "column_headers",       &_getters::column_headers },
      { "columns",              &_getters::columns },
      { "has_corner_button",    &_getters::has_corner_button },
      { "min_column_width",     &_getters::min_column_width },
      { "rows",                 &_getters::rows },
      { "selection",            &_getters::selection },
      { "selection_mode",       &_getters::selection_mode },
      { "selection_type",       &_getters::selection_type },
      { "show_column_headers",  &_getters::show_column_headers },
      { "show_grid",            &_getters::show_grid },
      { "show_row_headers",     &_getters::show_row_headers },
      { "sortable",             &_getters::sortable },
      { "word_wrap",            &_getters::word_wrap },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "alternate_row_colors", &_setters::alternate_row_colors },
      { "column_headers",       &_setters::column_headers },
      { "has_corner_button",    &_setters::has_corner_button },
      { "min_column_width",     &_setters::min_column_width },
      { "selection_mode",       &_setters::selection_mode },
      { "selection_type",       &_setters::selection_type },
      { "show_column_headers",  &_setters::show_column_headers },
      { "show_grid",            &_setters::show_grid },
      { "show_row_headers",     &_setters::show_row_headers },
      { "sortable",             &_setters::sortable },
      { "word_wrap",            &_setters::word_wrap },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      define_collection_metatable(L, {
         .registry_key          = cls::row_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::rows::get_collection_length,
         .lookup_item_by_index   = &_collections::rows::lookup_item_by_index,
      });
      define_collection_metatable(L, {
         .registry_key          = cls::col_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::cols::get_collection_length,
         .lookup_item_by_index   = &_collections::cols::lookup_item_by_index,
      });
   }

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}