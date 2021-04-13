#include "dropdown.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"
#include "../../collections.h"

#include <QSortFilterProxyModel>
#include "../../ui/util/lua_item_model.h"

#include "../../cross_thread_tasks/s2m/lambda.h"

#include "../../../helpers/lua/qt_variant.h"

#pragma region Collection: "items"
namespace {
   using namespace editor_script;

   namespace _collections::items {
      using cls        = wrappers::ui::dropdown;
      using model_t    = ObservableStandardItemModel;
      using observer_t = DovahKitScriptVMItemModelObserver;

      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, cls::item_collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", cls::item_collection_key);
         }
         if (self->widget == nullptr) {
            luaL_error(L, "function called with zombie self (expected %s)", cls::item_collection_key);
         }
         return *self;
      }
      model_t& get_model(const wrapper& w) {
         auto* proxy = (QSortFilterProxyModel*) ((QComboBox*)w.widget)->model();
         assert(proxy);
         auto* model = (model_t*) proxy->sourceModel();
         assert(model);
         return *model;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self  = get_collection_wrapper(L);
         auto& model = get_model(self);
         lua_pushinteger(L, model.rowCount());
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self  = get_collection_wrapper(L);
         auto& model = get_model(self);
         auto  row   = lua_tointeger(L, 2);
         //
         auto* root  = model.invisibleRootItem();
         auto* item  = root->child(row);
         if (!item)
            return 0;
         observer_t* observer = nullptr;
         //
         {
            auto* task = new tasks::s2m::lambda(true);
            task->handler = [&self, row, &observer]() {
               auto& model = get_model(self);
               auto* root  = model.invisibleRootItem();
               auto* item  = root->child(row);
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
         wrapper iw = self;
         iw.model_observer = observer;
         return DovahKitScriptVMUserdataInterface::get().push(L, iw, wrappers::ui::dropdown_item::metatable_key);
      }
   }
}
#pragma endregion

#pragma region dropdown
namespace widget_lua {
   using namespace editor_script;
   using cls = wrappers::ui::dropdown;
   using wrapped_type = cls::wrapped_type;
   
   ObservableStandardItemModel& get_model(QComboBox* w) {
      auto* proxy = (QSortFilterProxyModel*) w->model();
      assert(proxy);
      auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
      assert(model);
      return *model;
   }

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t items(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.is_collection = true;
         out.into_collection(wrapper_part_types::ui_dropdown_items);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::item_collection_key);
      }
      luastackchange_t selected_index(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = -1;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = widget->currentIndex(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushinteger(L, result);
         return 1;
      }
      luastackchange_t selected_item(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         DovahKitScriptVMItemModelObserver* result = nullptr;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() {
               int   pos   = widget->currentIndex();
               auto* proxy = (QSortFilterProxyModel*) widget->model();
               auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
               auto* root  = model->invisibleRootItem();
               //
               auto  r_qmi = root->index(); // returns an invalid index
               auto  p_qmi = proxy->mapFromSource(r_qmi);
               auto  i_qmi = proxy->index(pos, 0, p_qmi);
               auto  m_qmi = proxy->mapToSource(i_qmi);
               //
               auto* item  = root->child(m_qmi.row());
               if (!item)
                  return;
               result = model->getOrCreateRegisteredObserver(model->indexFromItem(item));
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (!result)
            return 0;
         //
         wrapper iw = self;
         iw.model_observer = result;
         return DovahKitScriptVMUserdataInterface::get().push(L, iw, wrappers::ui::dropdown_item::metatable_key);
      }
      luastackchange_t selected_text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result;
         {
            auto* widget = (wrapped_type*)self.widget;
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = widget->currentText(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t selected_index(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         int   isnum;
         int   value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer expected");
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         task->handler = [widget, value]() {
            if (value >= widget->count())
               widget->setCurrentIndex(-1);
            else
               widget->setCurrentIndex(value);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t selected_text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         auto  value   = QString::fromUtf8(lua_tostring(L, 2));
         task->handler = [widget, value]() { widget->setCurrentText(value); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.dropdown.new function should not be called with a colon or passed any arguments");
         //
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created]() {
            created = new wrapped_type();
            //
            auto* model = new ObservableStandardItemModel(created);
            auto* proxy = new QSortFilterProxyModel(created);
            proxy->setSourceModel(model);
            created->setModel(proxy);
            //
            DovahKitScriptVM::get().set_up_new_scripted_widget(created);
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
   /*static*/ const std::initializer_list<luaL_Reg> widget_lua::cls::metatable_methods = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> widget_lua::cls::metatable_getters = {
      { "items",          &widget_lua::_getters::items },
      { "selected_index", &widget_lua::_getters::selected_index },
      { "selected_text",  &widget_lua::_getters::selected_text },
   };
   /*static*/ const std::initializer_list<luaL_Reg> widget_lua::cls::metatable_setters = {
      { "selected_index", &widget_lua::_setters::selected_index },
      { "selected_text",  &widget_lua::_setters::selected_text },
   };

   /*static*/ void widget_lua::cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      editor_script::define_class(L, metatable_key, nullptr, metatable_methods);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &widget_lua::_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &widget_lua::_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setfield(L, pos, widget_lua::cls::global_name);
      //
      // Set up collection:
      //
      editor_script::define_collection_metatable(L, {
         .registry_key          = widget_lua::cls::item_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::items::get_collection_length,
         .lookup_item_by_index   = &_collections::items::lookup_item_by_index,
      });
   }
}
#pragma endregion

#pragma region dropdown_item
namespace item_lua {
   using namespace editor_script;
   using cls = wrappers::ui::dropdown_item;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t data(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget || !self.model_observer)
            return 0;
         QVariant result;
         {
            auto* observer = self.model_observer;
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [observer, &result]() {
               if (auto* item = observer->item())
                  result = item->data(Qt::UserRole);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         return DovahKitScriptVM::get().push_to_lua(result);
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget || !self.model_observer)
            return 0;
         QString result;
         {
            auto* observer = self.model_observer;
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [observer, &result]() {
               if (auto* item = observer->item())
                  result = item->data(Qt::DisplayRole).toString();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t data(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto& vm    = DovahKitScriptVM::get();
         auto  value = vm.variant_from_lua(2);
         if (!value.isValid() && !lua_isnoneornil(L, 2)) {
            luaL_error(L, "the provided value cannot be stored as a dropdown item's data member");
         }
         if (!self.widget || !self.model_observer)
            return 0;
         auto* observer = self.model_observer;
         auto* task     = new tasks::s2m::lambda(false);
         task->handler  = [observer, value]() {
            if (auto* item = observer->item())
               item->setData(value, Qt::UserRole);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget || !self.model_observer)
            return 0;
         auto* observer = self.model_observer;
         auto* task     = new tasks::s2m::lambda(false);
         auto  value    = QString::fromUtf8(lua_tostring(L, 2));
         task->handler  = [observer, value]() {
            if (auto* item = observer->item())
               item->setData(value, Qt::DisplayRole);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
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
   /*static*/ const std::initializer_list<luaL_Reg> item_lua::cls::metatable_methods = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> item_lua::cls::metatable_getters = {
      { "data", &item_lua::_getters::data },
      { "text", &item_lua::_getters::text },
   };
   /*static*/ const std::initializer_list<luaL_Reg> item_lua::cls::metatable_setters = {
      { "data",  &item_lua::_setters::data },
      { "text",  &item_lua::_setters::text },
   };

   /*static*/ void item_lua::cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      editor_script::define_class(L, metatable_key, nullptr, metatable_methods);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &item_lua::_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setfield(L, pos, item_lua::cls::global_name);
   }
}
#pragma endregion