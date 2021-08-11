#include "events.h"
#include "coordinator.h"
#include "userdata.h"
#include "../../../lua.h"
#include "../../../helpers/lua/isempty.h"
#include "../../../helpers/lua/qt_variant.h"
#include "../../../helpers/lua/set_top_on_exit.h"
#include "../../../helpers/qt/combobox.h"
#include "../../../helpers/qt/get_model_of.h"
#include "../verify_threading.h"
#include "../../push_native_object.h"
#include "../../safe_call.h"
#include "../../wrapper.h"

#include "events/script_event.h"

#include "../../../editor/form_stub_meta_type.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTabWidget>
#include "../../../ui/generic/FormPicker.h"

#include "events/registration/button.h"
#include "events/registration/checkbox.h"
#include "events/registration/dropdown.h"
#include "events/registration/formpicker.h"
#include "events/registration/groupbox.h"
#include "events/registration/radio_button.h"
#include "events/registration/radio_group.h"
#include "events/registration/spinbox.h"
#include "events/registration/tabbox.h"
#include "events/registration/table_view.h"
#include "events/registration/textbox.h"

namespace {
   static constexpr const char* listener_registry_key = "dovahscript.internals.event_listeners";
}

namespace dovahscript::impl {
   extern QObject& get_event_connection_recipient() {
      return core::subsystems::coordinator::get();
   }
}

namespace {
   struct _event_list_for_target_type {
      const QMetaObject* const meta;
      std::vector<const char*> events;
      
      _event_list_for_target_type(const QMetaObject* const m, std::initializer_list<const char*> e) : meta(m), events(e) {}
   };
   std::array _events_by_target_type = {
      _event_list_for_target_type(&FormPicker::staticMetaObject,
         dovahscript::impl::event_registration::formpicker::event_names
      ),
      _event_list_for_target_type(&QButtonGroup::staticMetaObject,
         dovahscript::impl::event_registration::radio_group::event_names
      ),
      _event_list_for_target_type(&QCheckBox::staticMetaObject,
         dovahscript::impl::event_registration::checkbox::event_names
      ),
      _event_list_for_target_type(&QComboBox::staticMetaObject,
         dovahscript::impl::event_registration::dropdown::event_names
      ),
      _event_list_for_target_type(&QDoubleSpinBox::staticMetaObject,
         dovahscript::impl::event_registration::spinbox::event_names
      ),
      _event_list_for_target_type(&QGroupBox::staticMetaObject,
         dovahscript::impl::event_registration::groupbox::event_names
      ),
      _event_list_for_target_type(&QLineEdit::staticMetaObject,
         dovahscript::impl::event_registration::textbox::event_names
      ),
      _event_list_for_target_type(&QPushButton::staticMetaObject,
         dovahscript::impl::event_registration::button::event_names
      ),
      _event_list_for_target_type(&QRadioButton::staticMetaObject,
         dovahscript::impl::event_registration::radio_button::event_names
      ),
      _event_list_for_target_type(&QTableView::staticMetaObject,
         dovahscript::impl::event_registration::table_view::event_names
      ),
      _event_list_for_target_type(&QTabWidget::staticMetaObject,
         dovahscript::impl::event_registration::tabbox::event_names
      ),
   };

   bool event_name_is_valid(const QObject& widget, const char* event_name) {
      auto* mo = widget.metaObject();
      for (auto& entry : _events_by_target_type) {
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
         dovahscript::core::subsystems::events::get().receive_event_from_main_thread(this->widget, this->event_name.c_str(), this->listener_name.c_str(), { QVariant::fromValue<Args>(args)... });
      }
   };
}

namespace dovahscript::core::subsystems {
   void events::on_script_setup() {
      assert(this->pending_events.empty());
      assert(this->pending_event_count == 0);
   }
   void events::on_script_teardown() {
      this->pending_events.clear();
      this->pending_event_count = 0;
   }

   size_t events::process_pending_events() {
      require_script_thread();
      return this->pending_events.process();
   }

   void events::abandon_object(QObject& target) {
      require_client_thread();
      //
      auto descendants = target.findChildren<QObject*>();
      bool had_any_connections;
      //
      had_any_connections = QObject::disconnect(&target, nullptr, &impl::get_event_connection_recipient(), nullptr);
      if (had_any_connections)
         this->pending_events.forget_about(target);
      //
      for (auto* d : descendants) {
         had_any_connections = QObject::disconnect(d, nullptr, &impl::get_event_connection_recipient(), nullptr);
         if (had_any_connections)
            this->pending_events.forget_about(*d);
      }
   }


   void events::_connect_event(passkey_to<impl::event_registration::base>, QMetaObject::Connection connection, QObject& target, const char* event_name, const char* listener_name) {
      require_script_thread();
      //
      auto& entry = this->connections[&target][event_name][listener_name];
      QObject::disconnect(entry); // replace the existing listener, if any
      entry = connection;
   }
   void events::_register_event(QObject& widget, const char* event_name, const char* listener_name) {
      require_script_thread();
      //
      static constexpr std::array registrars = {
         &impl::event_registration::button::register_event,
         &impl::event_registration::checkbox::register_event,
         &impl::event_registration::dropdown::register_event,
         &impl::event_registration::formpicker::register_event,
         &impl::event_registration::groupbox::register_event,
         &impl::event_registration::radio_button::register_event,
         &impl::event_registration::radio_group::register_event,
         &impl::event_registration::spinbox::register_event,
         &impl::event_registration::tabbox::register_event,
         &impl::event_registration::table_view::register_event,
         &impl::event_registration::textbox::register_event,
      };
      for (auto* registrar : registrars) {
         using result_t = impl::event_registration::result;
         //
         auto result = (registrar)(widget, event_name, listener_name);
         assert(result != result_t::failure);
         if (result != result_t::no_match)
            return;
      }
      //
      assert(false && "Unknown event target!");
   }

   void events::add_listener(QObject& widget, const char* event_name, const char* listener_name, int listener_index) {
      require_script_thread();
      if (!event_name_is_valid(widget, event_name))
         return;
      //
      auto* L     = coordinator::get().lua_state;
      auto  start = lua_gettop(L);
      //
      auto  si_storage = start + 1;
      auto  si_events  = start + 2;
      auto  si_funcs   = start + 3;
      //
      lua_getfield(L, LUA_REGISTRYINDEX, listener_registry_key);
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
   void events::remove_listener(QObject& widget, const char* event_name, const char* listener_name) {
      require_script_thread();
      //
      auto* L     = coordinator::get().lua_state;
      auto  start = lua_gettop(L);
      //
      auto  si_storage = start + 1;
      auto  si_events  = start + 2;
      //
      lua_getfield(L, LUA_REGISTRYINDEX, listener_registry_key); // push 1
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
         auto& events = this->connections[&widget][event_name];
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
   void events::remove_all_listeners(QObject& widget) {
      require_script_thread();
      //
      auto* L     = coordinator::get().lua_state;
      auto  start = lua_gettop(L);
      //
      lua_getfield(L, LUA_REGISTRYINDEX, listener_registry_key);
      lua_pushlightuserdata(L, &widget);
      lua_pushnil(L);
      lua_rawset(L, start + 1);
      //
      widget.disconnect();
      //
      lua_settop(L, start);
   }
   void events::fire_event(QObject& widget, const char* event_name, const char* listener_name, const std::vector<QVariant>& params) {
      require_script_thread();
      constexpr bool double_check_stack = false;
      //
      auto* L     = coordinator::get().lua_state;
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
      lua_getfield(L, LUA_REGISTRYINDEX, listener_registry_key);
      // STACK: - [ ..., storage ] +
      if constexpr (double_check_stack) {
         assert(lua_gettop(L)   == si_storage);
         assert(lua_type(L, -1) == LUA_TTABLE);
      }
      lua_pushlightuserdata(L, &widget);
      if (lua_rawget(L, si_storage) != LUA_TTABLE) { // no listeners for this widget
         --this->pending_event_count;
         return;
      }
      // STACK: - [ ..., storage, storage[&widget] ] +
      if constexpr (double_check_stack) {
         assert(lua_gettop(L) == si_events);
      }
      if (lua_getfield(L, si_events, event_name) != LUA_TTABLE) {
         --this->pending_event_count;
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
      if constexpr (double_check_stack) {
         assert(lua_gettop(L) == si_temp);
      }
      int count = 0;
      //
      auto& userdata_intfc = userdata::get();
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
            if (ut == qMetaTypeId<impl::model_observer_event_argument>()) {
               using namespace dovahscript;
               //
               auto arg = p.value<impl::model_observer_event_argument>();
               assert(arg.observer);
               assert(arg.metatable_key && arg.metatable_key[0]);
               wrapper out;
               out.type = wrapper_type::model_observer;
               out.model_observer = arg.observer;
               argcount += userdata_intfc.push(L, out, arg.metatable_key);
               continue;
            }
            if (ut == qMetaTypeId<dovah::form_stub*>()) { // the generic helper functions can't handle any DovahKit-specific types
               auto* stub = p.value<dovah::form_stub*>();
               if (stub) {
                  argcount += push_native_object(stub);
               } else {
                  lua_pushnil(L);
                  ++argcount;
               }
               continue;
            }
            if (ut == qMetaTypeId<QObject*>()) {
               auto* value = p.value<QObject*>();
               auto* arg   = qobject_cast<QWidget*>(value);
               if (arg) {
                  argcount += push_native_object(arg);
                  continue;
               }
            }
            argcount += cobb::lua::push_qt_variant(L, p);
         }
         safe_call(L, argcount, 0); // pops the called function
      }
      --this->pending_event_count;
   }

   void events::receive_event_from_main_thread(QObject& widget, const char* event_name, const char* listener_name, const std::vector<QVariant>& params) {
      ++this->pending_event_count;
      //
      // Called by the main thread; sends a message to the script thread.
      //
      auto* task  = new impl::script_event(widget, event_name, listener_name, params);
      this->pending_events.push_back(task);
   }
}