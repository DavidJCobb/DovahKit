#include "table_view.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"
#include "../../collections.h"

#include <QHeaderView>
#include <QSortFilterProxyModel>
#include "../../ui/util/lua_item_model.h"

#include "../../cross_thread_tasks/s2m/lambda.h"

#include "helpers/model_observer_data.h"
#include "helpers/widget_properties.h"

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

   void _extract_cell_arg_list(lua_State* L, int first, QVector<editor_script::moph::handler_set::role_map_t>& roles, QVector<QStandardItem*>& items) {
      using namespace editor_script;

      auto& core = DovahKitScriptVMCore::get();
      int   argcount = lua_gettop(L);
      for (int i = first; i <= argcount; ++i) {
         if (auto* ia = wrapper_from_stack<wrappers::ui::table_view_cell>(L, i)) {
            assert(ia->model_observer);
            items.push_back(ia->model_observer->item());
            roles.push_back(moph::handler_set::role_map_t());
            continue;
         }
         auto type = lua_type(L, i);
         if (type == LUA_TTABLE) {
            auto e = wrappers::ui::table_view_cell::moph_handlers.extract(L, i);
            roles.push_back(e);
            items.push_back(nullptr);
            continue;
         }
         moph::handler_set::role_map_t e;
         e[Qt::DisplayRole] = core.variant_from_lua(i);
         roles.push_back(e);
         items.push_back(nullptr);
      }
      assert(roles.size() == items.size());
   }
}

#pragma region Collection: "rows"
namespace {
   using namespace editor_script;

   namespace _collections::rows {
      using cls        = wrappers::ui::table_view;
      using model_t    = ObservableStandardItemModel;
      using observer_t = ObservableStandardItemModelObserver;

      static constexpr auto collection_key = cls::row_collection_key;

      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", collection_key);
         }
         if (self->widget == nullptr) {
            luaL_error(L, "function called with zombie self (expected %s)", collection_key);
         }
         return *self;
      }
      model_t& get_model(cls::wrapped_type* widget) {
         auto* proxy = (QSortFilterProxyModel*)widget->model();
         assert(proxy);
         auto* model = (model_t*)proxy->sourceModel();
         assert(model);
         return *model;
      }
      model_t& get_model(const wrapper& w) {
         return get_model((cls::wrapped_type*)w.widget);
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         int   count;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto* widget  = (cls::wrapped_type*) self.widget;
            task->handler = [widget, &count]() {
               auto& model = get_model(widget);
               count = model.rowCount();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushinteger(L, count);
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self  = get_collection_wrapper(L);
         auto& model = get_model(self);
         auto  row   = lua_tointeger(L, 2) - 1; // lua indices start from one, not zero
         if (row < 0)
            return 0;
         //
         observer_t* observer = nullptr;
         {
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [&self, row, &observer]() {
               auto& model = get_model(self);
               if (row < 0 || row >= model.rowCount())
                  return;
               observer = model.getOrCreateRegisteredObserver(QModelIndex(), model_t::rowOrientation, row);
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
         return DovahKitScriptVMUserdataInterface::get().push(L, iw, wrappers::ui::table_view_row::metatable_key);
      }
   }
}
#pragma endregion

#pragma region Collection: "columns"
namespace {
   using namespace editor_script;

   namespace _collections::cols {
      using cls        = wrappers::ui::table_view;
      using model_t    = ObservableStandardItemModel;
      using observer_t = ObservableStandardItemModelObserver;

      static constexpr auto collection_key = cls::col_collection_key;

      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", collection_key);
         }
         if (self->widget == nullptr) {
            luaL_error(L, "function called with zombie self (expected %s)", collection_key);
         }
         return *self;
      }
      model_t& get_model(cls::wrapped_type* widget) {
         auto* proxy = (QSortFilterProxyModel*) widget->model();
         assert(proxy);
         auto* model = (model_t*) proxy->sourceModel();
         assert(model);
         return *model;
      }
      model_t& get_model(const wrapper& w) {
         return get_model((cls::wrapped_type*)w.widget);
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         int   count;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto* widget  = (cls::wrapped_type*) self.widget;
            task->handler = [widget, &count]() {
               auto& model = get_model(widget);
               count = model.columnCount();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushinteger(L, count);
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self  = get_collection_wrapper(L);
         auto& model = get_model(self);
         auto  col   = lua_tointeger(L, 2) - 1; // lua indices start from one, not zero
         if (col < 0)
            return 0;
         //
         observer_t* observer = nullptr;
         {
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [&self, col, &observer]() {
               auto& model = get_model(self);
               if (col < 0 || col >= model.columnCount())
                  return;
               observer = model.getOrCreateRegisteredObserver(QModelIndex(), model_t::colOrientation, col);
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
         return DovahKitScriptVMUserdataInterface::get().push(L, iw, wrappers::ui::table_view_col::metatable_key);
      }
   }
}
#pragma endregion

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::table_view;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      using role_map_t = moph::handler_set::role_map_t;

      luastackchange_t append_column(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         //
         // This function is designed in mimicry of the append-row column.
         //
         QVector<role_map_t>     roles;
         QVector<QStandardItem*> items;
         _extract_cell_arg_list(L, 2, roles, items);
         //
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget, roles, items]() { // do NOT pass args by reference, as this lambda is set not to block, so it'll go out of scope if you do!
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            //
            assert(roles.size() == items.size());
            int size = roles.size();
            QList<QStandardItem*> to_append;
            to_append.reserve(size);
            for (int i = 0; i < size; ++i) {
               auto* item = items[i];
               auto& data = roles[i];
               if (!item) {
                  item = new QStandardItem();
                  for (auto it = data.begin(); it != data.end(); ++it)
                     item->setData(it.value(), it.key());
               }
               to_append.push_back(item);
            }
            model->appendColumn(to_append);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         //
         return 0;
      }
      luastackchange_t append_row(lua_State* L) {
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
         QVector<role_map_t>     roles;
         QVector<QStandardItem*> items;
         _extract_cell_arg_list(L, 2, roles, items);
         //
         // We've extracted the Lua arguments. Now, let's pass them in.
         //
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget, roles, items]() { // do NOT pass (args) by reference, as this lambda is set not to block, so it'll go out of scope if you do!
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            if (model->rowCount() >= max_wrappable_word_count) // safety measure for large tables
               disable_word_wrap_without_changing_truncation(widget);
            //
            assert(roles.size() == items.size());
            int size = roles.size();
            QList<QStandardItem*> to_append;
            to_append.reserve(size);
            for (int i = 0; i < size; ++i) {
               auto* item = items[i];
               auto& data = roles[i];
               if (!item) {
                  item = new QStandardItem();
                  for (auto it = data.begin(); it != data.end(); ++it)
                     item->setData(it.value(), it.key());
               }
               to_append.push_back(item);
            }
            model->appendRow(to_append);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         //
         return 0;
      }
      luastackchange_t clear(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         helpers::remove_items_from_model(self.widget, -2, -2);
         return 0;
      }
      luastackchange_t insert_column(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int isnum;
         int insert_at = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer expected");
         if (--insert_at < 0) {
            luaL_argerror(L, 2, "column numbers start at 1");
         }
         //
         QVector<role_map_t>     roles;
         QVector<QStandardItem*> items;
         _extract_cell_arg_list(L, 3, roles, items);
         //
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(true);
         task->handler = [widget, roles, items, insert_at]() mutable { // do NOT pass args by reference, as this lambda is set not to block, so it'll go out of scope if you do!
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            //
            assert(roles.size() == items.size());
            int size = roles.size();
            QList<QStandardItem*> to_append;
            to_append.reserve(size);
            for (int i = 0; i < size; ++i) {
               auto* item = items[i];
               auto& data = roles[i];
               if (!item) {
                  item = new QStandardItem();
                  for (auto it = data.begin(); it != data.end(); ++it)
                     item->setData(it.value(), it.key());
               }
               to_append.push_back(item);
            }
            int cc = model->columnCount();
            if (insert_at > cc)
               insert_at = cc;
            model->insertColumn(insert_at, to_append);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         return 0;
      }
      luastackchange_t insert_row(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int isnum;
         int insert_at = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer expected");
         if (--insert_at < 0) {
            luaL_argerror(L, 2, "row numbers start at 1");
         }
         //
         QVector<role_map_t>     roles;
         QVector<QStandardItem*> items;
         _extract_cell_arg_list(L, 3, roles, items);
         //
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(true);
         task->handler = [widget, roles, items, insert_at]() mutable { // do NOT pass args by reference, as this lambda is set not to block, so it'll go out of scope if you do!
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            if (model->rowCount() >= max_wrappable_word_count) // safety measure for large tables
               disable_word_wrap_without_changing_truncation(widget);
            //
            assert(roles.size() == items.size());
            int size = roles.size();
            QList<QStandardItem*> to_append;
            to_append.reserve(size);
            for (int i = 0; i < size; ++i) {
               auto* item = items[i];
               auto& data = roles[i];
               if (!item) {
                  item = new QStandardItem();
                  for (auto it = data.begin(); it != data.end(); ++it)
                     item->setData(it.value(), it.key());
               }
               to_append.push_back(item);
            }
            int rc = model->rowCount();
            if (insert_at > rc)
               insert_at = rc;
            model->insertRow(insert_at, to_append);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         return 0;
      }
      luastackchange_t remove_column(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int   isnum;
         int   col  = lua_tointegerx(L, 2, &isnum);
         if (!isnum) {
            auto* wrapper = wrapper_from_stack<wrappers::ui::table_view_col>(L, 2);
            luaL_argcheck(L, wrapper != nullptr, 2, "expected an integer or table_view_col");
            if (!wrapper->model_observer)
               return 0;
            col = wrapper->model_observer->col;
            if (col == -1)
               return 0;
         } else {
            luaL_argcheck(L, col > 0, 2, "table view columns are numbered from 1");
         }
         if (!self.widget)
            return 0;
         --col;
         helpers::remove_items_from_model(self.widget, -2, col);
         return 0;
      }
      luastackchange_t remove_row(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int   isnum;
         int   row = lua_tointegerx(L, 2, &isnum);
         if (!isnum) {
            auto* wrapper = wrapper_from_stack<wrappers::ui::table_view_row>(L, 2);
            luaL_argcheck(L, wrapper != nullptr, 2, "expected an integer or table_view_row");
            if (!wrapper->model_observer)
               return 0;
            row = wrapper->model_observer->row;
            if (row == -1)
               return 0;
         } else {
            luaL_argcheck(L, row > 0, 2, "table view rows are numbered from 1");
         }
         if (!self.widget)
            return 0;
         --row;
         helpers::remove_items_from_model(self.widget, row, -2);
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t alternate_row_colors(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::alternatingRowColors);
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t column_headers(lua_State* L) {
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
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_createtable(L, headers.size(), 0);
         for (int i = 0; i < headers.size(); ++i) {
            lua_pushstring(L, headers[i].toUtf8());
            lua_rawseti(L, -2, i + 1);
         }
         return 1;
      }
      luastackchange_t columns(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_table_view_cols;
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::col_collection_key);
      }
      luastackchange_t has_corner_button(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::isCornerButtonEnabled);
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t rows(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_table_view_rows;
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::row_collection_key);
      }
      luastackchange_t selection(lua_State* L) {
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
                     metatable_key = editor_script::wrappers::ui::table_view_row::metatable_key;
                     for (auto& qmi : sm->selectedRows()) {
                        auto  remapped = proxy->mapToSource(qmi);
                        auto* o        = model->getOrCreateRegisteredObserver(QModelIndex(), ObservableStandardItemModel::rowOrientation, remapped.row());
                        assert(o);
                        observers.push_back(o);
                     }
                     break;
                  case QAbstractItemView::SelectionBehavior::SelectColumns:
                     metatable_key = editor_script::wrappers::ui::table_view_col::metatable_key;
                     for (auto& qmi : sm->selectedColumns()) {
                        auto  remapped = proxy->mapToSource(qmi);
                        auto* o        = model->getOrCreateRegisteredObserver(QModelIndex(), ObservableStandardItemModel::colOrientation, remapped.column());
                        assert(o);
                        observers.push_back(o);
                     }
                     break;
                  case QAbstractItemView::SelectionBehavior::SelectItems:
                     metatable_key = editor_script::wrappers::ui::table_view_cell::metatable_key;
                     for (auto& qmi : sm->selectedIndexes()) {
                        auto  remapped = proxy->mapToSource(qmi);
                        auto* o        = model->getOrCreateRegisteredObserver(remapped);
                        assert(o);
                        observers.push_back(o);
                     }
                     break;
               }
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         size_t size = observers.size();
         lua_createtable(L, size, 0);
         int tbl = lua_gettop(L);
         for (size_t i = 0; i < size; ++i) {
            wrapper iw;
            iw.type = wrapper_type::ui_model_item;
            iw.model_observer = observers[i];
            int count = DovahKitScriptVMUserdataInterface::get().push(L, iw, metatable_key);
            if (count) {
               lua_rawseti(L, tbl, i + 1);
               if (count > 1)
                  lua_pop(L, count - 1);
            }
         }
         return 1;
      }
      luastackchange_t selection_mode(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         auto result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::selectionMode);
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
      luastackchange_t selection_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         auto result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::selectionBehavior);
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
      luastackchange_t show_column_headers(lua_State* L) {
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
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t show_grid(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::showGrid);
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t show_row_headers(lua_State* L) {
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
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t sortable(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QTableView::isSortingEnabled);
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t word_wrap(lua_State* L) {
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
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
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
      luastackchange_t alternate_row_colors(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setAlternatingRowColors, value);
         return 0;
      }
      luastackchange_t column_headers(lua_State* L) {
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
            luaL_argcheck(L, isnum, 2, "argument's length operator did not return an integer");
         }
         QStringList text;
         text.reserve(count);
         for (int i = 1; i <= count; ++i) {
            lua_geti(L, 2, i);
            text.push_back(lua_tostring(L, 3));
            lua_pop(L, 1);
         }
         auto* task    = new tasks::s2m::lambda(false);
         auto* widget  = (wrapped_type*)self.widget;
         task->handler = [widget, text]() {
            auto* proxy = (QSortFilterProxyModel*)widget->model();
            auto* model = (ObservableStandardItemModel*)proxy->sourceModel();
            model->setHorizontalHeaderLabels(text);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t has_corner_button(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setCornerButtonEnabled, value);
         return 0;
      }
      luastackchange_t selection_mode(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
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
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setSelectionMode, value);
         return 0;
      }
      luastackchange_t selection_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
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
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setSelectionBehavior, value);
         return 0;
      }
      luastackchange_t show_column_headers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto  value   = lua_toboolean(L, 2);
         auto* task    = new tasks::s2m::lambda(false);
         auto* widget  = (wrapped_type*) self.widget;
         task->handler = [widget, value]() {
            widget->horizontalHeader()->setHidden(!value);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t show_grid(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setShowGrid, value);
         return 0;
      }
      luastackchange_t show_row_headers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto  value   = lua_toboolean(L, 2);
         auto* task    = new tasks::s2m::lambda(false);
         auto* widget  = (wrapped_type*) self.widget;
         task->handler = [widget, value]() {
            widget->verticalHeader()->setHidden(!value);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t sortable(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QTableView::setSortingEnabled, value);
         return 0;
      }
      luastackchange_t word_wrap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
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
         auto* task    = new tasks::s2m::lambda(false);
         auto* widget  = (wrapped_type*)self.widget;
         task->handler = [widget, ww]() {
            set_word_wrapping_for(widget, ww);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.table_view.new function should not be called with a colon or passed any arguments");
         //
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created]() {
            created = new wrapped_type();
            created->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
            created->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows); // sensible defaults
            created->setCornerButtonEnabled(false); // sensible defaults
            auto* vh = created->verticalHeader();
            vh->setDefaultSectionSize(0); // get rid of weird padding
            vh->setHidden(true); // sensible defaults
            //
            DovahKitScriptVMCore::get().set_up_widget_model(created);
            DovahKitScriptVMCore::get().set_up_new_scripted_widget(created);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         wrapper out;
         auto* mt = wrap_widget(out, created);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace editor_script::wrappers::ui {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "append_column", &_methods::append_column },
      { "append_row",    &_methods::append_row },
      { "clear",         &_methods::clear },
      { "insert_column", &_methods::insert_column },
      { "insert_row",    &_methods::insert_row },
      { "remove_column", &_methods::remove_column },
      { "remove_row",    &_methods::remove_row },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "alternate_row_colors", &_getters::alternate_row_colors },
      { "column_headers",       &_getters::column_headers },
      { "columns",              &_getters::columns },
      { "has_corner_button",    &_getters::has_corner_button },
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
      { "selection_mode",       &_setters::selection_mode },
      { "selection_type",       &_setters::selection_type },
      { "show_column_headers",  &_setters::show_column_headers },
      { "show_grid",            &_setters::show_grid },
      { "show_row_headers",     &_setters::show_row_headers },
      { "sortable",             &_setters::sortable },
      { "word_wrap",            &_setters::word_wrap },
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      editor_script::define_class(L, metatable_key, nullptr, metatable_methods);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setfield(L, pos, cls::global_name);
      //
      // Set up collection:
      //
      editor_script::define_collection_metatable(L, {
         .registry_key          = cls::row_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::rows::get_collection_length,
         .lookup_item_by_index   = &_collections::rows::lookup_item_by_index,
      });
      editor_script::define_collection_metatable(L, {
         .registry_key          = cls::col_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::cols::get_collection_length,
         .lookup_item_by_index   = &_collections::cols::lookup_item_by_index,
      });
   }
}