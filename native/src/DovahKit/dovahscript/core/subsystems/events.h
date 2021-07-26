#pragma once
#include <string>
#include <unordered_map>
#include <QObject>
#include "../../../helpers/singleton.h"

class ObservableStandardItemModelObserver;

namespace dovahscript::impl {
   struct model_observer_event_argument {
      ObservableStandardItemModelObserver* observer = nullptr;
      const char* metatable_key = nullptr;
   };
}

namespace dovahscript::core::subsystems {
   class events : cobb::singleton {
      protected:
         events() {
            qRegisterMetaType<dovahscript::impl::model_observer_event_argument>();
         }
      public:
         static events& get() {
            static events instance;
            return instance;
         }

      protected:
         // connections[q_object][event_name][listener] = connection;
         std::unordered_map<QObject*, std::unordered_map<std::string, std::unordered_map<std::string, QMetaObject::Connection>>> connections;

      public:
         unsigned int pending_event_count() const noexcept;

         // Call from the script thread's idle loop. Returns the number of events processed.
         size_t process_pending_events();

         // Completely disconnects an object from the event system, and then discards all pending events for 
         // the object. This should be called by the lifetime subsystem just before deleting an object.
         static_assert(false, "TODO: The lifetime subsystem should call this when deleting an object.");
         void abandon_object(QObject&);

         void add_listener(QObject& target, const char* event_name, const char* listener_name, int listener_index);
         void remove_listener(QObject& target, const char* event_name, const char* listener_name = nullptr);
         void remove_all_listeners(QObject& target);

         // The script thread calls this in response to the main thread firing an evnet.
         void fire_event(QObject& target, const char* event_name, const char* listener_name, const std::vector<QVariant>& params);

         void receive_event_from_main_thread(QObject& target, const char* event_name, const char* listener_name, const std::vector<QVariant>& params);

      protected:
         // Helper function for forwarding the arguments of a Qt signal into Lua verbatim. There are a limited 
         // number of cases where the templates don't resolve properly for unknown reasons, and this can result 
         // in arguments not being forwarded, so if you see that happening you'll just have to specify the 
         // template arguments manually.
         template<class target_t, class signal_context_t, typename... Args> void _connect_event(target_t& target, void(signal_context_t::* signal)(Args...), const char* event_name, const char* listener_name) {
            auto& entry = this->connections[(QObject*)&target][event_name][listener_name];
            QObject::disconnect(entry);
            entry = QObject::connect(&target, signal, &vm, _event_forwarding_lambda<Args...>(target, event_name, listener_name), Qt::DirectConnection);
         }

         // Helper function for wiring a Qt signal into Lua, if you've set up the QObject connection yourself. 
         // Doing it yourself allows you to specify custom arguments for Lua.
         void _connect_event(QMetaObject::Connection connection, QObject& target, const char* event_name, const char* listener_name);

         // Basically a glorified switch-case pyramid, to call (_connect_event) with the right Qt signal.
         void _register_event(QObject& target, const char* event_name, const char* listener_name);
   };
}

// IntelliSense doesn't like Q_DECLARE_METATYPE; ignore errors here unless they're compiler errors:
Q_DECLARE_METATYPE(dovahscript::impl::model_observer_event_argument)
// needed so that QObject::connect can pass these across threads (by copying them). refer to DovahKitCore's constructor as well.