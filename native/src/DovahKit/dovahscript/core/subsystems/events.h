#pragma once
#include <string>
#include <unordered_map>
#include <QObject>
#include <QVariant>
#include "helpers/passkey.h"
#include "helpers/singleton.h"
#include "../../../lua.h"
#include "./events/script_event_queue.h"

struct ObservableStandardItemModelObserver;
namespace dovahscript::core::subsystems {
   class events;
}

namespace dovahscript::impl {
   //
   // Having all event-related signal/slot connections use the same recipient means that we can 
   // quickly and easily disconnect all event-related connections from a QObject all at once.
   //
   extern QObject& get_event_connection_recipient();

   struct model_observer_event_argument {
      ObservableStandardItemModelObserver* observer = nullptr;
      const char* metatable_key = nullptr;
   };

   namespace event_registration {
      class base;
   }

   // Custom lambda struct, used as the slot handler for the Qt signal/slot connections we create 
   // when routing Qt events into Lua.
   template<typename... Args> struct event_forwarding_lambda {
      event_forwarding_lambda(QWidget& w, const char* n, const char* l) : widget(w), event_name(n), listener_name(l) {}

      QWidget& widget;
      const std::string event_name;
      const std::string listener_name;

      void operator()(Args... args) {
         core::subsystems::events::get().receive_event_from_main_thread(this->widget, this->event_name.c_str(), this->listener_name.c_str(), { QVariant::fromValue<Args>(args)... });
      }
   };
}

namespace dovahscript::core::subsystems {
   class events : cobb::singleton {
      protected:
         events() {
            qRegisterMetaType<dovahscript::impl::model_observer_event_argument>();
         }

         template<typename B> using passkey_to = cobb::passkey<events, B>;

      public:
         static events& get() {
            static events instance;
            return instance;
         }

      protected:
         // connections[q_object][event_name][listener] = connection;
         std::unordered_map<QObject*, std::unordered_map<std::string, std::unordered_map<std::string, QMetaObject::Connection>>> connections;
         impl::script_event_queue pending_events;
         unsigned int pending_event_count = 0;

      public:
         unsigned int get_pending_event_count() const noexcept;

         void initialize(lua_State*);
         void on_script_teardown();

         // Call from the script thread's idle loop. Returns the number of events processed.
         size_t process_pending_events();

         // Completely disconnects an object from the event system, and then discards all pending events for 
         // the object and its descendants. This should be called by the lifetime subsystem just before 
         // deleting an object.
         void abandon_object(QObject&);

         void add_listener(QObject& target, const char* event_name, const char* listener_name, int listener_index);
         void remove_listener(QObject& target, const char* event_name, const char* listener_name = nullptr);
         void remove_all_listeners(QObject& target);

         // The script thread calls this in response to the main thread firing an evnet.
         void fire_event(QObject& target, const char* event_name, const char* listener_name, const std::vector<QVariant>& params);

         void receive_event_from_main_thread(QObject& target, const char* event_name, const char* listener_name, const std::vector<QVariant>& params);

      public:
         // Helper function for forwarding the arguments of a Qt signal into Lua verbatim. There are a limited 
         // number of cases where the templates don't resolve properly for unknown reasons, and this can result 
         // in arguments not being forwarded, so if you see that happening you'll just have to specify the 
         // template arguments manually.
         template<class target_t, class signal_context_t, typename... Args> void _connect_event(
            passkey_to<impl::event_registration::base>,
            target_t& target,
            void(signal_context_t::* signal)(Args...),
            const char* event_name,
            const char* listener_name
         ) {
            auto& entry = this->connections[(QObject*)&target][event_name][listener_name];
            QObject::disconnect(entry);
            entry = QObject::connect(&target, signal, &impl::get_event_connection_recipient(), impl::event_forwarding_lambda<Args...>(target, event_name, listener_name), Qt::DirectConnection);
         }

         // Helper function for wiring a Qt signal into Lua, if you've set up the QObject connection yourself. 
         // Doing it yourself allows you to specify custom arguments for Lua.
         void _connect_event(
            passkey_to<impl::event_registration::base>,
            QMetaObject::Connection connection,
            QObject& target,
            const char* event_name,
            const char* listener_name
         );

      protected:
         // Basically a glorified switch-case pyramid, to call (_connect_event) with the right Qt signal.
         void _register_event(QObject& target, const char* event_name, const char* listener_name);
   };
}

// IntelliSense doesn't like Q_DECLARE_METATYPE; ignore errors here unless they're compiler errors:
Q_DECLARE_METATYPE(dovahscript::impl::model_observer_event_argument)
// needed so that QObject::connect can pass these across threads (by copying them). refer to DovahKitCore's constructor as well.