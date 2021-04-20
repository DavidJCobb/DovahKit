#include "ui_listeners.h"
#include "userdata.h"
#include "editor_script_inner_core.h"

#include "../../form_stub_meta_type.h" // needed for QVariants of form stub pointers

#include "../../../helpers/lua/isempty.h"
#include "../../../helpers/lua/qt_variant.h"
#include "../../../helpers/lua/set_top_on_exit.h"
#include "../wrapper_util.h"

#include "../../../ui/generic/FormPicker.h"
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTableView>
#include "../../../helpers/qt/combobox.h" // for events

#include <QSortFilterProxyModel>
#include "../wrappers/ui/table_view/cell.h"
#include "../wrappers/ui/table_view/col.h"
#include "../wrappers/ui/table_view/row.h"

namespace {
   struct _event_widget {
      const QMetaObject* const meta;
      std::vector<const char*> events;
      //
      _event_widget(const QMetaObject* const m, std::initializer_list<const char*> e) : meta(m), events(e) {}
   };
   std::array _events_by_widget = {
      _event_widget(&FormPicker::staticMetaObject,
         {
            "OnChanged",
         }
      ),
      _event_widget(&QCheckBox::staticMetaObject,
         {
            "OnChanged",
            "OnToggled", // The same as OnChanged, but the argument is a boolean indicating whether the checkbox is checked.
         }
      ),
      _event_widget(&QComboBox::staticMetaObject,
         {
            "OnChanged", // The dropdown's selected logical index was changed through some cause other than the script directly setting it or the selected text.
         }
      ),
      _event_widget(&QDoubleSpinBox::staticMetaObject,
         {
            "OnChanged", // The spinbox's value has been altered by the user. Fires instantly for increment/decrement buttons; for typing, works like the textbox OnChanged event.
         }
      ),
      _event_widget(&QGroupBox::staticMetaObject,
         {
            "OnToggled",
         }
      ),
      _event_widget(&QLineEdit::staticMetaObject,
         {
            "OnChanged",       // The textbox's value was previously altered, and the user hit Enter or moved focus away from the textbox.
            "OnInputRejected", // The textbox rejected input because it didn't validate or the max length would've been exceeded.
            "OnKeyPressed",    // The textbox's value was altered by a keypress.
         }
      ),
      _event_widget(&QPushButton::staticMetaObject,
         {
            "OnActivated",         // The button was clicked (or interacted with analogously via another input device).
            "OnCheckStateChanged", // The button is checkable and its check state changed.
         }
      ),
      _event_widget(&QTableView::staticMetaObject,
         {
            "OnSelectionChanged",
         }
      ),
   };

   bool event_name_is_valid(const QWidget& widget, const char* event_name) {
      auto* mo = widget.metaObject();
      for (auto& entry : _events_by_widget) {
         if (!mo->inherits(entry.meta))
            continue;
         for (auto* name : entry.events) {
            if (_stricmp(event_name, name) == 0)
               return true;
         }
         return false;
      }
      return false;
   }

   // Custom lambda struct, used as the slot handler for the Qt signal/slot connections we create 
   // when routing Qt events into Lua.
   template<typename... Args> struct _event_forwarding_lambda {
      _event_forwarding_lambda(QWidget& w, const char* n, const char* l) : widget(w), event_name(n), listener_name(l) {}

      QWidget& widget;
      const std::string event_name;
      const std::string listener_name;

      void operator()(Args... args) {
         //
         // Runs on the main thread.
         //
         DovahKitScriptUIListenerInterface::get().receive_event_from_main_thread(this->widget, this->event_name.c_str(), this->listener_name.c_str(), { QVariant::fromValue<Args>(args)... });
      }
   };
}
void DovahKitScriptUIListenerInterface::_connect_event(QMetaObject::Connection connection, QWidget& widget, const char* event_name, const char* listener_name) {
   DovahKitScriptVMCore::require_script_thread();
   auto& vm    = DovahKitScriptVMCore::get();
   auto& entry = vm.widgets.connections[&widget][event_name][listener_name];
   QObject::disconnect(entry); // replace the existing listener, if any
   entry = connection;
}
void DovahKitScriptUIListenerInterface::_register_event(QWidget& widget, const char* event_name, const char* listener_name) {
   DovahKitScriptVMCore::require_script_thread();
   //
   // Runs on the script thread.
   //
   auto& vm = DovahKitScriptVMCore::get();
   if (auto* casted = qobject_cast<FormPicker*>(&widget)) {
      if (_stricmp(event_name, "OnChanged") == 0) {
         this->_connect_event(*casted, &FormPicker::formChanged, event_name, listener_name);
         return;
      }
   } else if (auto* casted = qobject_cast<QCheckBox*>(&widget)) {
      if (_stricmp(event_name, "OnChanged") == 0) {
         std::string ln = listener_name;
         this->_connect_event(
            QObject::connect(casted, &QCheckBox::stateChanged, &vm,
               [casted, ln](int state) {
                  QString s;
                  switch (state) {
                     case Qt::CheckState::Checked:
                        s = "checked";
                        break;
                     case Qt::CheckState::PartiallyChecked:
                        s = "indeterminate";
                        break;
                     case Qt::CheckState::Unchecked:
                        s = "unchecked";
                        break;
                  }
                  DovahKitScriptUIListenerInterface::get().receive_event_from_main_thread(*casted, "OnChanged", ln.c_str(), { s });
               }
            ),
            widget, event_name, listener_name
         );
         return;
      }
      if (_stricmp(event_name, "OnToggled") == 0) {
         std::string ln = listener_name;
         this->_connect_event(
            QObject::connect(casted, &QCheckBox::stateChanged, &vm,
               [casted, ln](int state) {
                  DovahKitScriptUIListenerInterface::get().receive_event_from_main_thread(*casted, "OnToggled", ln.c_str(), { state == Qt::CheckState::Checked });
               }
            ),
            widget, event_name, listener_name
         );
         return;
      }
   } else if (auto* casted = qobject_cast<QComboBox*>(&widget)) {
      if (_stricmp(event_name, "OnChanged") == 0) {
         std::string ln = listener_name;
         this->_connect_event(
            QObject::connect(casted, QOverload<int>::of(&QComboBox::currentIndexChanged), &vm,
               [casted, ln](int index) {
                  auto* proxy   = casted->model();
                  int   logical = cobb::qt::map_combobox_index_from_proxy(casted, index); // map proxy combobox index to logical combobox index
                  ++logical; // Lua is one-indexed, not zero-indexed
                  DovahKitScriptUIListenerInterface::get().receive_event_from_main_thread(*casted, "OnChanged", ln.c_str(), { logical });
               }
            ),
            widget, event_name, listener_name
         );
         return;
      }
   } else if (auto* casted = qobject_cast<QDoubleSpinBox*>(&widget)) {
      if (_stricmp(event_name, "OnChanged") == 0) {
         this->_connect_event(*casted, QOverload<double>::of(&QDoubleSpinBox::valueChanged), event_name, listener_name);
         return;
      }
   } else if (auto* casted = qobject_cast<QGroupBox*>(&widget)) {
      if (_stricmp(event_name, "OnToggled") == 0) {
         this->_connect_event(*casted, &QGroupBox::toggled, event_name, listener_name);
         return;
      }
   } else if (auto* casted = qobject_cast<QLineEdit*>(&widget)) {
      if (_stricmp(event_name, "OnChanged") == 0) {
         std::string ln = listener_name;
         this->_connect_event(
            QObject::connect(casted, &QLineEdit::editingFinished, &vm,
               [casted, ln]() {
                  DovahKitScriptUIListenerInterface::get().receive_event_from_main_thread(*casted, "OnChanged", ln.c_str(), { casted->text() });
               }
            ),
            widget, event_name, listener_name
         );
         return;
      }
      if (_stricmp(event_name, "OnInputRejected") == 0) {
         this->_connect_event(*casted, &QLineEdit::inputRejected, event_name, listener_name);
         return;
      }
      if (_stricmp(event_name, "OnKeyPressed") == 0) {
         this->_connect_event(*casted, &QLineEdit::textEdited, event_name, listener_name);
         return;
      }
   } else if (auto* casted = qobject_cast<QPushButton*>(&widget)) {
      if (_stricmp(event_name, "OnActivated") == 0) {
         this->_connect_event(*casted, &QPushButton::clicked, event_name, listener_name);
         return;
      }
      if (_stricmp(event_name, "OnCheckStateChanged") == 0) {
         this->_connect_event(*casted, &QPushButton::toggled, event_name, listener_name);
         return;
      }
   } else if (auto* casted = qobject_cast<QTableView*>(&widget)) {
      if (_stricmp(event_name, "OnSelectionChanged") == 0) {
         std::string ln = listener_name;
         this->_connect_event(
            QObject::connect(casted->selectionModel(), &QItemSelectionModel::selectionChanged, &vm,
               [casted, ln]() {
                  //
                  // We can't easily tell from the signal alone whether the selection is supposed to be a row, 
                  // a column, or a cell, so we'll just check the widget itself to find out.
                  //
                  std::vector<QVariant> selections;
                  {
                     auto* sm    = casted->selectionModel();
                     auto* proxy = (QSortFilterProxyModel*) casted->model();
                     auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
                     switch (casted->selectionBehavior()) {
                        case QAbstractItemView::SelectionBehavior::SelectRows:
                           for (auto& qmi : sm->selectedRows()) {
                              auto remapped = proxy->mapToSource(qmi);
                              //
                              LuaModelObserverEventArgument arg;
                              arg.observer      = model->getOrCreateRegisteredObserver(QModelIndex(), ObservableStandardItemModel::rowOrientation, remapped.row());
                              arg.metatable_key = editor_script::wrappers::ui::table_view_row::metatable_key;
                              assert(arg.observer);
                              selections.push_back(QVariant::fromValue(arg));
                           }
                           break;
                        case QAbstractItemView::SelectionBehavior::SelectColumns:
                           for (auto& qmi : sm->selectedColumns()) {
                              auto remapped = proxy->mapToSource(qmi);
                              //
                              LuaModelObserverEventArgument arg;
                              arg.observer      = model->getOrCreateRegisteredObserver(QModelIndex(), ObservableStandardItemModel::colOrientation, remapped.column());
                              arg.metatable_key = editor_script::wrappers::ui::table_view_col::metatable_key;
                              assert(arg.observer);
                              selections.push_back(QVariant::fromValue(arg));
                           }
                           break;
                        case QAbstractItemView::SelectionBehavior::SelectItems:
                           for (auto& qmi : sm->selectedIndexes()) {
                              auto remapped = proxy->mapToSource(qmi);
                              //
                              LuaModelObserverEventArgument arg;
                              arg.observer      = model->getOrCreateRegisteredObserver(remapped);
                              arg.metatable_key = editor_script::wrappers::ui::table_view_cell::metatable_key;
                              assert(arg.observer);
                              selections.push_back(QVariant::fromValue(arg));
                           }
                           break;
                     }
                  }
                  DovahKitScriptUIListenerInterface::get().receive_event_from_main_thread(*casted, "OnSelectionChanged", ln.c_str(), selections);
               }
            ),
            widget, event_name, listener_name
         );
         return;
      }
   }
}

void DovahKitScriptUIListenerInterface::add_listener(QWidget& widget, const char* event_name, const char* listener_name, int listener_index) {
   DovahKitScriptVMCore::require_script_thread();
   if (!event_name_is_valid(widget, event_name))
      return;
   //
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   //
   auto  si_storage = start + 1;
   auto  si_events  = start + 2;
   auto  si_funcs   = start + 3;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::ui_listener_registry_key);
   assert(lua_type(L, -1) == LUA_TTABLE);
   // STACK: - [ ..., storage_root ] +
   lua_pushlightuserdata(L, &widget);
   lua_rawget(L, si_storage);
   // STACK: - [ ..., storage_root, storage_root[&widget] ] +
   if (lua_isnoneornil(L, si_events)) {
      lua_pop(L, 1);
      lua_createtable(L, 0, 1);
      lua_pushlightuserdata(L, &widget);
      lua_pushvalue(L, si_events);
      lua_rawset(L, si_storage);
   }
   // STACK: - [ ..., storage_root, storage_root[&widget] ] +
   lua_getfield(L, si_events, event_name);
   // STACK: - [ ..., storage_root, storage_root[&widget], storage_root[&widget][event_name] ] +
   bool empty = lua_isnoneornil(L, si_funcs);
   if (empty) {
      lua_pop(L, 1);
      lua_createtable(L, 0, 1);
      lua_pushstring (L, event_name);
      lua_pushvalue  (L, si_funcs);
      lua_rawset(L, si_events);
      //
      this->_register_event(widget, event_name, listener_name);
   }
   lua_pushstring(L, listener_name);
   lua_pushvalue (L, listener_index);
   lua_rawset(L, si_funcs);
   //
   lua_settop(L, start);
}
void DovahKitScriptUIListenerInterface::remove_listener(QWidget& widget, const char* event_name, const char* listener_name) {
   DovahKitScriptVMCore::require_script_thread();
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   //
   auto  si_storage = start + 1;
   auto  si_events  = start + 2;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::ui_listener_registry_key); // push 1
   lua_pushlightuserdata(L, &widget);
   lua_rawget(L, si_storage); // STACK: - [ ..., storage_root, storage_root[&widget] ] +
   assert(lua_istable(L, -1));
   lua_copy  (L, -1, si_storage);
   lua_settop(L, si_storage); // STACK: - [ ..., storage_root[&widget] ] +
   lua_getfield(L, si_storage, event_name); // STACK: - [ ..., storage_root[&widget], storage_root[&widget][event_name] ] +
   if (lua_isnoneornil(L, si_events)) { // no events to remove
      lua_settop(L, start);
      return;
   }
   bool empty = true;
   if (listener_name) {
      empty = false;
      lua_pushnil(L);
      lua_setfield(L, si_events, listener_name);
      empty = cobb::lua::isempty(L, si_events);
   }
   if (empty) { // no listeners left for this event. disconnect the Qt signal for it
      lua_pushnil(L);
      lua_setfield(L, si_storage, event_name);
      //
      // Disconnect the signal:
      //
      auto& events = vm.widgets.connections[&widget][event_name];
      #pragma warning(suppress: 6387) // listener_name should never be nullptr, so don't bother me about it
      auto  it     = events.find(listener_name);
      if (it != events.end()) {
         QObject::disconnect(it->second);
         events.erase(it);
      }
   }
   //
   lua_settop(L, start);
}
void DovahKitScriptUIListenerInterface::remove_all_listeners(QWidget& widget) {
   DovahKitScriptVMCore::require_script_thread();
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::ui_listener_registry_key);
   lua_pushlightuserdata(L, &widget);
   lua_pushnil(L);
   lua_rawset(L, start + 1);
   //
   widget.disconnect();
   //
   lua_settop(L, start);
}
void DovahKitScriptUIListenerInterface::fire_event(QWidget& widget, const char* event_name, const char* listener_name, const std::vector<QVariant>& params) {
   DovahKitScriptVMCore::require_script_thread();
   constexpr bool double_check_stack = false;
   //
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   auto  guard = cobb::lua::set_top_on_exit(L, start);
   //
   auto  si_storage = start + 1;
   auto  si_events  = start + 2;
   auto  si_funcs   = start + 3;
   auto  si_temp    = start + 4;
   auto  si_nk      = start + 5;
   auto  si_nv      = start + 6;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::ui_listener_registry_key);
   // STACK: - [ ..., storage ] +
   if (double_check_stack) {
      assert(lua_gettop(L)   == si_storage);
      assert(lua_type(L, -1) == LUA_TTABLE);
   }
   lua_pushlightuserdata(L, &widget);
   if (lua_rawget(L, si_storage) != LUA_TTABLE) { // no listeners for this widget
      --this->vm.pending_ui_event_count;
      return;
   }
   // STACK: - [ ..., storage, storage[&widget] ] +
   if (double_check_stack) {
      assert(lua_gettop(L) == si_events);
   }
   if (lua_getfield(L, si_events, event_name) != LUA_TTABLE) {
      --this->vm.pending_ui_event_count;
      return;
   }
   //
   // Let's create a temporary table to hold the listener functions we want to execute. Why? 
   // Because the player could potentially register more listeners *from* a listener, and if 
   // we're just directly executing listeners as we iterate over them with lua_next, then 
   // their doing so will break lua_next. Instead, we'll iterate to grab the listeners by 
   // name, and stuff them into an array.
   //
   lua_createtable(L, 0, 0);
   if (double_check_stack) {
      assert(lua_gettop(L) == si_temp);
   }
   int count = 0;
   //
   auto& userdata_intfc = DovahKitScriptVMUserdataInterface::get();
   // STACK: - [ ..., storage, storage[&widget], storage[&widget][event_name] ] +
   lua_pushnil(L); // nk
   while (lua_next(L, si_funcs) != 0) {
      // STACK: - [ ..., storage, storage[&widget], storage[&widget][event_name], key, value ] +
      lua_pushinteger(L, ++count);
      lua_rotate(L, -2, 1);   // STACK: - [ ..., funcs, temp, key, count, value ] +
      lua_rawset(L, si_temp); // STACK: - [ ..., funcs, temp, key ] +
   }
   // STACK: - [ ..., funcs, temp ] +
   //
   // Now let's execute the listeners.
   //
   for (int i = 0; i < count; ++i) {
      lua_rawgeti(L, si_temp, i + 1);
      int argcount = 0;
      for (auto& p : params) {
         auto ut = p.userType();
         if (ut == qMetaTypeId<LuaModelObserverEventArgument>()) {
            using namespace editor_script;
            //
            auto arg = p.value<LuaModelObserverEventArgument>();
            assert(arg.observer);
            assert(arg.metatable_key && arg.metatable_key[0]);
            wrapper out;
            out.type = wrapper_type::ui_model_item;
            out.model_observer = arg.observer;
            argcount += userdata_intfc.push(L, out, arg.metatable_key);
            continue;
         }
         if (ut == qMetaTypeId<dovah::form_stub*>()) { // the generic helper functions can't handle any DovahKit-specific types
            auto* stub = p.value<dovah::form_stub*>();
            if (stub) {
               using namespace editor_script;
               wrapper out;
               auto* mt  = wrap_form(out, stub);
               argcount += userdata_intfc.push(L, out, mt);
            } else {
               lua_pushnil(L);
               ++argcount;
            }
            continue;
         }
         argcount += cobb::lua::push_qt_variant(L, p);
      }
      editor_script::util::safe_call(L, argcount, 0); // pops the called function
   }
   --this->vm.pending_ui_event_count;
}

void DovahKitScriptUIListenerInterface::receive_event_from_main_thread(QWidget& widget, const char* event_name, const char* listener_name, const std::vector<QVariant>& params) {
   ++this->vm.pending_ui_event_count;
   //
   // Called by the main thread; sends a message to the script thread.
   //
   auto* task  = new editor_script::ui_event(widget, event_name, listener_name, params);
   DovahKitScriptVMCore::get().ui_queues.events.push_back(task);
}