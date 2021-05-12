#pragma once
#include <vector>
#include <QModelIndex>
#include <QWidget>
#include "editor_script_inner_core.h"
#include "../ui/util/lua_item_model.h"

struct LuaModelObserverEventArgument {
   ObservableStandardItemModelObserver* observer = nullptr;
   const char* metatable_key = nullptr;
};

class DovahKitScriptUIListenerInterface : cobb::singleton {
   protected:
      DovahKitScriptUIListenerInterface(DovahKitScriptVMCore& w) : vm(w) {
         qRegisterMetaType<LuaModelObserverEventArgument>();
      }
   public:
      static DovahKitScriptUIListenerInterface& get() {
         static DovahKitScriptUIListenerInterface instance(DovahKitScriptVMCore::get());
         return instance;
      }
      
      DovahKitScriptVMCore& vm;

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
         auto& vm    = this->vm;
         auto& entry = vm.widgets.connections[(QObject*)&target][event_name][listener_name];
         QObject::disconnect(entry);
         entry = QObject::connect(&target, signal, &vm, _event_forwarding_lambda<Args...>(target, event_name, listener_name));
      }

      // Helper function for wiring a Qt signal into Lua, if you've set up the QObject connection yourself. 
      // Doing it yourself allows you to specify custom arguments for Lua.
      void _connect_event(QMetaObject::Connection connection, QObject& target, const char* event_name, const char* listener_name);

      // Basically a glorified switch-case pyramid, to call (_connect_event) with the right Qt signal.
      void _register_event(QObject& target, const char* event_name, const char* listener_name);
};

// IntelliSense doesn't like Q_DECLARE_METATYPE; ignore errors here unless they're compiler errors:
Q_DECLARE_METATYPE(LuaModelObserverEventArgument)
// needed so that QObject::connect can pass these across threads (by copying them). refer to DovahKitCore's constructor as well.