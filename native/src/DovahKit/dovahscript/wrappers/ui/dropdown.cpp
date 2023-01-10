#include "dropdown.h"
#include <QSortFilterProxyModel>
#include "../../../helpers/lua/error.h"
#include "../../../helpers/qt/combobox.h"
#include "../../../ui/generic/ObservableStandardItemModel.h"
#include "../../core/subsystems/lifetime.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/resources/DovahscriptResourceStyledItemDelegate.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"
#include "../../wrapper.h"

#include "../../tasks/s2m/create_ui_widget.h"
#include "../../tasks/s2m/ui_read_lambda.h"
#include "../../tasks/s2m/ui_write_lambda.h"

#include "../../api_helpers/model_observers.h"
#include "../../api_helpers/model_observer_property_handlers.h"
#include "../../api_helpers/qt_variant.h"
#include "../../api_helpers/widget_properties.h"

#include "dropdown/collection_items.h"
#include "dropdown/item.h"
#include "../resource/raster.h"

/*
   
   The dropdowns that we provide to scripts support sorting by way of a QSortFilterProxyModel. 
   Scripts always work with and see dropdown items' "logical indices," not their "proxied indices." 
   For example, given the following unsorted list:

      1  Ari
      2  Chris
      3  Brianna
      4  Quigley
      5  Lucrezia

   If the list were to be sorted via the proxy model, the proxied indices would become:

      1  Ari
      2  Brianna
      3  Chris
      4  Lucrezia
      5  Quigley

   However, Lucrezia's logical index would remain 5, and if she were selected, the script would be 
   told that the selected index is 5.

   We therefore need to translate indices from proxied to logical whenever we return them to Lua.

*/

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::dropdown;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      using role_map_t = api_helpers::moph::handler_set::role_map_t;

      int append_item(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         role_map_t roles;
         if (lua_gettop(L) >= 2) {
            if (lua_type(L, 2) == LUA_TTABLE) {
               roles = wrappers::ui::dropdown_item::moph_handlers.extract(L, 2);
            } else {
               roles[Qt::DisplayRole] = api_helpers::pull_variant(L, 2);
            }
         }
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, roles]() { // do NOT pass (roles) by reference, as this lambda is set not to block, so it'll go out of scope if you do!
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            auto* item  = new QStandardItem();
            for (auto it = roles.begin(); it != roles.end(); ++it)
               item->setData(it.value(), it.key());
            model->appendRow(item);
         };
         send_script_ui_task(*task);
         //
         return 0;
      }
      int clear(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         api_helpers::remove_items_from_model(self.widget, -2, -2);
         return 0;
      }
      int map_logical_index_to_proxy(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int isnum;
         int input = lua_tointegerx(L, 2, &isnum);
         cobb::lua::argcheck(L, isnum, 2, "integer expected");
         if (--input < 0)
            cobb::lua::argerror(L, 2, "items are numbered from 1");
         //
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_read_lambda();
         int   result  = 0;
         task->handler = [widget, input, &result]() {
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            auto  qmi   = model->index(input, 0);
            result = proxy->mapFromSource(qmi).row();
         };
         send_script_ui_task(*task);
         delete task;
         //
         ++result;
         if (result > 0)
            lua_pushinteger(L, result);
         else
            lua_pushnil(L);
         return 1;
      }
      int map_proxy_index_to_logical(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int isnum;
         int input = lua_tointegerx(L, 2, &isnum);
         cobb::lua::argcheck(L, isnum, 2, "integer expected");
         if (--input < 0)
            cobb::lua::argerror(L, 2, "items are numbered from 1");
         //
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_read_lambda;
         int   result  = 0;
         task->handler = [widget, input, &result]() {
            auto* proxy = (QSortFilterProxyModel*) widget->model();
            auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
            auto  qmi   = proxy->index(input, 0);
            result = proxy->mapToSource(qmi).row();
         };
         send_script_ui_task(*task);
         delete task;
         //
         ++result;
         if (result > 0)
            lua_pushinteger(L, result);
         else
            lua_pushnil(L);
         return 1;
      }
      int remove_item(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         //
         // Allow the script to pass in a numeric logical index or a userdata-wrapper for a 
         // dropdown item.
         //
         int index = -1;
         ObservableStandardItemModelObserver* observer = nullptr;
         if (lua_type(L, 2) == LUA_TUSERDATA) {
            auto* w = wrapper_from_stack<wrappers::ui::dropdown_item>(L, 2);
            cobb::lua::argcheck(L, w, 2, "integer index or item expected");
            observer = w->model_observer;
         } else {
            int isnum;
            index = lua_tointegerx(L, 2, &isnum);
            cobb::lua::argcheck(L, isnum, 2, "integer index or item expected");
            cobb::lua::argcheck(L, index > 0, 2, "indices must be greater than zero");
            --index;
         }
         //
         if (!self.widget)
            return 0;
         auto  widget   = task_reference((wrapped_type*) self.widget);
         auto* task     = new tasks::s2m::ui_write_lambda(true); // removals must block
         bool  mismatch = false;
         task->handler = [widget, index, observer, &mismatch]() {
            auto* proxy = (QSortFilterProxyModel*)widget->model();
            auto* model = (ObservableStandardItemModel*)proxy->sourceModel();
            //
            int row = index;
            QStandardItem* item = nullptr;
            if (observer) {
               if (observer->model != model) {
                  mismatch = true;
                  return;
               }
               item = observer->item();
               if (!item)
                  return;
               row = item->row();
            } else {
               item = model->item(row);
            }
            if (!item)
               return;
            auto* parent = item->parent();
            if (!parent) {
               parent = model->invisibleRootItem();
               assert(parent);
            }
            //
            // Before we remove the item, we need to block signals so that QComboBox::currentIndexChanged 
            // does not fire. I haven't been able to figure out exactly why, but QSortFilterProxyModel's 
            // mapToSource function doesn't work properly during the process of removing something from a 
            // combobox model. It works just fine before and after the removal, but not during. I checked 
            // its source code and the most I can grasp is that it listens for the source model's signals, 
            // including rowsAboutToBeRemoved and rowsRemoved; it does work to update its mappings in both 
            // signals, but when it catches rowsAboutToBeRemoved, it emits its own rowsRemoved; so perhaps 
            // when it's used in a QComboBox, the combobox updates in response to the QSFPM tasks being 
            // only partway done. Dunno.
            //
            // Anyway, if we allow a signal to be emitted naturally, then we'll convert the proxied index 
            // to a logical index incorrectly due to the unknown defect in mapToSource, and we'll then 
            // pass that incorrect logical index to Lua. Not good!
            //
            {
               const auto blocker = QSignalBlocker(widget);
               parent->removeRow(row);
            }
            emit widget->currentIndexChanged(widget->currentIndex()); // disgusting hack: emit the signal ourselves, after the removal, so that the Lua-facing event fires with correct info
         };
         send_script_ui_task(*task);
         delete task;
         //
         if (mismatch) {
            cobb::lua::argerror(L, 2, "the specified dropdown_item belongs to a different dropdown, not to this one");
         }
         //
         core::subsystems::lifetime::get().zombify_all_invalid_model_observers();
         //
         return 0;
      }
   }
   namespace _getters {
      int items(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_dropdown_items;
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::ui::collections::dropdown_items.registry_key);
      }
      int selected_index(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = -1;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda;
            task->handler = [widget, &result]() {
               result = cobb::qt::map_combobox_index_from_proxy(widget, widget->currentIndex());
            };
            send_script_ui_task(*task);
            delete task;
         }
         if (result < 0)
            lua_pushnil(L);
         else
            lua_pushinteger(L, result + 1); // Lua is one-indexed, not zero-indexed, so increment it
         return 1;
      }
      int selected_item(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         ObservableStandardItemModelObserver* result = nullptr;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda;
            task->handler = [widget, &result]() {
               int pos   = widget->currentIndex();
               int logic = cobb::qt::map_combobox_index_from_proxy(widget, pos);
               if (logic < 0)
                  return;
               //
               auto* proxy = (QSortFilterProxyModel*) widget->model();
               auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
               auto* root  = model->invisibleRootItem();
               auto* item  = root->child(logic);
               if (!item)
                  return;
               result = model->getOrCreateRegisteredObserver(model->indexFromItem(item));
            };
            send_script_ui_task(*task);
            delete task;
         }
         if (!result)
            return 0;
         //
         wrapper iw;
         iw.type = wrapper_type::model_observer;
         iw.model_observer = result;
         return core::subsystems::userdata::get().push(L, iw, wrappers::ui::dropdown_item::metatable_key);
      }
      int selected_text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QComboBox::currentText);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int sorted(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result;
         {
            auto* widget  = (wrapped_type*)self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() {
               auto* proxy = (QSortFilterProxyModel*)widget->model();
               result = proxy->sortColumn() >= 0;
            };
            send_script_ui_task(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
   }
   namespace _setters {
      int selected_index(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         int   isnum;
         int   value = lua_tointegerx(L, 2, &isnum);
         if (!isnum) {
            cobb::lua::argcheck(L, lua_isnoneornil(L, 2), 2, "integer or nil expected");
            value = 0;
         } else {
            cobb::lua::argcheck(L, value > 0, 2, "combobox indices cannot be zero or negative; to clear the selection, pass nil");
         }
         --value;
         if (!self.widget)
            return 0;
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, value]() {
            const auto blocker = QSignalBlocker(widget);
            if (value >= widget->count())
               widget->setCurrentIndex(-1);
            else
               widget->setCurrentIndex(value);
         };
         send_script_ui_task(*task);
         return 0;
      }
      int selected_text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         auto  value   = QString::fromUtf8(lua_tostring(L, 2));
         task->handler = [widget, value]() {
            const auto blocker = QSignalBlocker(widget);
            widget->setCurrentText(value);
         };
         send_script_ui_task(*task);
         return 0;
      }
      int sorted(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto  widget  = task_reference((wrapped_type*) self.widget);
         auto* task    = new tasks::s2m::ui_write_lambda(false);
         auto  value   = lua_toboolean(L, 2);
         task->handler = [widget, value]() {
            const auto blocker = QSignalBlocker(widget); // prevent QComboBox::currentIndexChanged, as we only want Lua to be notified when the logical selection changes, not the proxied selection
            auto* proxy = (QSortFilterProxyModel*)widget->model();
            proxy->sort(value ? 0 : -1); // using column -1 should restore default order
         };
         send_script_ui_task(*task);
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the ui.%s.new function should not be called with a colon or passed any arguments", cls::global_name);
         //
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
         task->configure = [](wrapped_type* created) {
            created->setItemDelegate(new DovahscriptResourceStyledItemDelegate(created));
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
      { "append_item",                &_methods::append_item },
      { "clear",                      &_methods::clear },
      { "map_logical_index_to_proxy", &_methods::map_logical_index_to_proxy },
      { "map_proxy_index_to_logical", &_methods::map_proxy_index_to_logical },
      { "remove_item",                &_methods::remove_item },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "items",          &_getters::items },
      { "selected_index", &_getters::selected_index },
      { "selected_item",  &_getters::selected_item },
      { "selected_text",  &_getters::selected_text },
      { "sorted",         &_getters::sorted },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "selected_index", &_setters::selected_index },
      { "selected_text",  &_setters::selected_text },
      { "sorted",         &_setters::sorted },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      define_collection_metatable(L, wrappers::ui::collections::dropdown_items);
   }

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}