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

      void add_listener(QWidget&, const char* event_name, const char* listener_name, int listener_index);
      void remove_listener(QWidget&, const char* event_name, const char* listener_name = nullptr);
      void remove_all_listeners(QWidget&);

      // The script thread calls this in response to the main thread firing an evnet.
      void fire_event(QWidget&, const char* event_name, const char* listener_name, const std::vector<QVariant>& params);

      void receive_event_from_main_thread(QWidget&, const char* event_name, const char* listener_name, const std::vector<QVariant>& params);

   protected:
      // Helper function for forwarding the arguments of a Qt signal into Lua verbatim. There are a limited 
      // number of cases where the templates don't resolve properly for unknown reasons, and this can result 
      // in arguments not being forwarded, so if you see that happening you'll just have to specify the 
      // template arguments manually.
      template<class widget_t, class signal_context_t, typename... Args> void _connect_event(widget_t& widget, void(signal_context_t::* signal)(Args...), const char* event_name, const char* listener_name) {
         auto& vm    = this->vm;
         auto& entry = vm.widgets.connections[(QWidget*)&widget][event_name][listener_name];
         QObject::disconnect(entry);
         entry = QObject::connect(&widget, signal, &vm, _event_forwarding_lambda<Args...>(widget, event_name, listener_name));
      }

      // Helper function for wiring a Qt signal into Lua, if you've set up the QObject connection yourself. 
      // Doing it yourself allows you to specify custom arguments for Lua.
      void _connect_event(QMetaObject::Connection connection, QWidget&, const char* event_name, const char* listener_name);

      // Basically a glorified switch-case pyramid, to call (_connect_event) with the right Qt signal.
      void _register_event(QWidget& widget, const char* event_name, const char* listener_name);
};

// IntelliSense doesn't like Q_DECLARE_METATYPE; ignore errors here unless they're compiler errors:
Q_DECLARE_METATYPE(LuaModelObserverEventArgument)
// needed so that QObject::connect can pass these across threads (by copying them). refer to DovahKitCore's constructor as well.