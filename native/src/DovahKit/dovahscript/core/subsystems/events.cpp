#include "events.h"
#include "coordinator.h"
#include "userdata.h"
#include "../../../lua.h"
#include "../../../helpers/lua/isempty.h"
#include "../../../helpers/lua/qt_variant.h"
#include "../../../helpers/lua/set_top_on_exit.h"
#include "../../../helpers/qt/combobox.h"
#include "../../../helpers/qt/get_model_of.h"
#include "../../../helpers/class_list.h"
#include "../verify_threading.h"
#include "../../push_native_object.h"
#include "../../safe_call.h"
#include "../../wrapper.h"

#include "events/script_event.h"

#include "../../../editor/form_stub_meta_type.h"
#include "../../api_helpers/qt_color.h"

#include "events/registration/button.h"
#include "events/registration/checkbox.h"
#include "events/registration/color_button.h"
#include "events/registration/dropdown.h"
#include "events/registration/formpicker.h"
#include "events/registration/groupbox.h"
#include "events/registration/radio_button.h"
#include "events/registration/radio_group.h"
#include "events/registration/spinbox.h"
#include "events/registration/tabbox.h"
#include "events/registration/table_view.h"
#include "events/registration/textarea.h"
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
   using _event_registrar_list = cobb::class_list<
      dovahscript::impl::event_registration::textarea,
      dovahscript::impl::event_registration::formpicker,
      dovahscript::impl::event_registration::radio_group,
      dovahscript::impl::event_registration::checkbox,
      dovahscript::impl::event_registration::color_button,
      dovahscript::impl::event_registration::dropdown,
      dovahscript::impl::event_registration::spinbox,
      dovahscript::impl::event_registration::groupbox,
      dovahscript::impl::event_registration::textbox,
      dovahscript::impl::event_registration::button,
      dovahscript::impl::event_registration::radio_button,
      dovahscript::impl::event_registration::table_view,
      dovahscript::impl::event_registration::tabbox//,
   >;

   namespace event_registrar_list_functors {
      template<typename T> struct event_name_is_valid {
         static bool execute(const QMetaObject* const m, const char* name, bool& matched) {
            if (&T::target_type::staticMetaObject != m)
               return false;
            for (auto* en : T::event_names) {
               if (_stricmp(name, en) == 0) {
                  matched = true;
                  return true;
               }
            }
            return true;
         }
      };

      template<typename T> struct attempt_event_registration {
         static bool execute(const QMetaObject* rtti, QObject& widget, const char* event_name, const char* listener_name, dovahscript::impl::event_registration::result& result) {
            using result_t = dovahscript::impl::event_registration::result;
            //
            if (&T::target_type::staticMetaObject != rtti)
               return false;
            result = T::register_event(widget, event_name, listener_name);
            assert(result != result_t::failure);
            return result != result_t::no_match; // stop iterating if there was a success or a failure
         }
      };
   }

   bool event_name_is_valid(const QObject& widget, const char* event_name) {
      bool matched = false;
      //
      const auto* rtti = widget.metaObject();
      do {
         _event_registrar_list::for_each_breakable_with_args<event_registrar_list_functors::event_name_is_valid>(rtti, event_name, matched);
         if (matched)
            break;
      } while (rtti = rtti->superClass());
      //
      return matched;
   }
}

namespace dovahscript::core::subsystems {
   unsigned int events::get_pending_event_count() const noexcept {
      return this->pending_event_count;
   }

   void events::initialize(lua_State* L) {
      assert(this->pending_events.empty());
      assert(this->pending_event_count == 0);
      lua_createtable(L, 0, 0);
      lua_setfield(L, LUA_REGISTRYINDEX, listener_registry_key);
   }
   void events::on_script_teardown() {
      this->pending_events.clear();
      this->pending_event_count = 0;
      this->connections.clear();
   }

   size_t events::process_pending_events() {
      require_script_thread();
      return this->pending_events.process();
   }

   void events::abandon_object(QObject& target) {
      require_client_thread();
      require_script_thread();
      //
      auto descendants = target.findChildren<QObject*>();
      bool had_any_connections;
      //
      this->remove_all_listeners(target);
      had_any_connections = QObject::disconnect(&target, nullptr, &impl::get_event_connection_recipient(), nullptr);
      if (had_any_connections)
         this->pending_events.forget_about(target);
      //
      for (auto* d : descendants) {
         this->remove_all_listeners(*d);
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
      using result_t = impl::event_registration::result;
      result_t result = result_t::no_match;
      //
      const auto* rtti = widget.metaObject();
      do {
         _event_registrar_list::for_each_breakable_with_args<event_registrar_list_functors::attempt_event_registration>(rtti, widget, event_name, listener_name, result);
         if (result != result_t::no_match)
            return;
      } while (rtti = rtti->superClass());
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
         auto& all_ev = this->connections[&widget];
         auto& events = all_ev[event_name];
         if (listener_name) {
            auto it = events.find(listener_name);
            if (it != events.end()) {
               QObject::disconnect(it->second);
               events.erase(it);
            }
         } else {
            for (auto& pair : events)
               QObject::disconnect(pair.second);
            all_ev.erase(event_name);
         }
      }
      //
      lua_settop(L, start);
   }
   void events::remove_all_listeners(QObject& widget) {
      require_script_thread();
      //
      auto& coordinator_s = coordinator::get();
      if (coordinator_s.teardown_in_progress())
         return;
      auto* L     = coordinator_s.lua_state;
      auto  start = lua_gettop(L);
      //
      lua_getfield(L, LUA_REGISTRYINDEX, listener_registry_key);
      lua_pushlightuserdata(L, &widget);
      lua_pushnil(L);
      lua_rawset(L, start + 1);
      //
      lua_settop(L, start);
      //
      widget.disconnect();
      auto it = this->connections.find(&widget);
      if (it != this->connections.end())
         this->connections.erase(it);
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
            if (ut == QMetaType::QColor) {
               api_helpers::push_color(L, p.value<QColor>());
               ++argcount;
               continue;
            }
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
               auto* arg   = qobject_cast<QWidget*>(value); // NOTE: This will need to be removed if we allow non-widget QObjects as event arguments.
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