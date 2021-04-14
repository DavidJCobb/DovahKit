#pragma once
#include "../../../Lua/lua.hpp"
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <QDialog>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QWidget>
#include "cross_thread_tasks/base.h"
#include "ui/event.h"
#include "ui/util/lua_item_model.h"
#include "wrapper.h"

namespace dovah {
   class form_stub;
}

class DovahKitScriptVMMessenger;
class DovahKitScriptVMUITaskConduit;
class DovahKitScriptVMPermissionInterface;
class DovahKitScriptVMUserdataInterface;
class DovahKitScriptUIListenerInterface;

class DovahKitScriptVM : public QObject {
   Q_OBJECT
   //
   // This is the core singleton for editor scripting. It is intended to be accessed directly by 
   // the main thread, and will run Lua scripts on a secondary thread. The singleton has its own 
   // Qt signal for the main thread event loop, and  will loop in the secondary thread as needed 
   // to block script execution  while waiting for any needed  information from the main thread.
   //
   friend class DovahKitScriptVMMessenger;
   friend class DovahKitScriptVMUITaskConduit;
   friend class DovahKitScriptVMPermissionInterface;
   friend class DovahKitScriptVMUserdataInterface;
   friend class DovahKitScriptUIListenerInterface;
   public:

      // Storage in the Lua registry for a cached copy of (string.format), which we place there 
      // when we start up the VM, to ensure that hardcoded functions that need that behavior can 
      // access it even if the Lua script tries to monkeypatch or replace its own copy.
      static constexpr const char* string_format_registry_key   = "cached:string.format";

      enum class ui_lock_override_state { unchanged, locked, unlocked };

   protected:
      DovahKitScriptVM();
      ~DovahKitScriptVM();

      struct _task_queue {
         using task = editor_script::cross_thread_task;
         //
         std::vector<task*> list;
         std::recursive_mutex lock;

         //
         // The receiving thread should use this function to execute tasks.
         //
         void process(int cap = std::numeric_limits<int>::max());

         //
         // Suitable only for use by the sending thread.
         //
         void wait_until_empty();

         void clear();
      };
      
      void _setup_lua_vm();
      void _teardown_lua_vm(); // can only safely run on the main thread, since it tears down Qt objects now too

      void _run_queued_functions(bool ui_locked);
      void _script_thread_loop();

      //
      // Returns (true) if the Lua VM should be kept alive even after the script has finished 
      // executing. This would be the case if there are any script-spawned UI windows that are 
      // still open and visible.
      //
      bool _should_keep_running() const noexcept;
      
      std::recursive_mutex exec_lock;
      std::atomic<bool> aborted = false; // main thread can set this to kill the script
      std::atomic<bool> running = false;
      struct {
         std::vector<ObservableStandardItemModelObserver*> pointers; // these vectors must be kept in synch
         std::vector<int> refcounts;
      } ui_model_observers;
      QWidget* ui_parent = nullptr;
      //
      struct {
         _task_queue s2m; // script-to-main
         struct { // main-to-script
            _task_queue urgent; // urgent messages. these MUST NOT trigger Lua code to execute!
         } m2s;
      } task_queues;
      //
      struct {
         std::vector<QDialog*> windows;
         std::vector<QWidget*> orphans;
         std::vector<QWidget*> pending_deletion;
         std::unordered_map<QWidget*, std::unordered_map<std::string, std::unordered_map<std::string, QMetaObject::Connection>>> connections; // connections[widget][event_name][listener] = connection;
      } widgets;
      struct {
         _task_queue read;  // script-to-main; always blocks
         _task_queue write; // script-to-main; may block
         editor_script::ui_event_queue events; // main-to-script
      } ui_queues;
      
   public:
      static DovahKitScriptVM& get() {
         static DovahKitScriptVM instance;
         return instance;
      }
      
      lua_State*  lua_vm = nullptr;
      std::thread thread;
      QTimer      main_thread_tick_timer;
      std::atomic<unsigned int> pending_ui_event_count = 0;
      ui_lock_override_state    ui_lock_override       = ui_lock_override_state::unchanged;
      
      inline bool is_aborted() const noexcept { return this->aborted; }
      inline bool is_running() const noexcept { return this->running; }

      inline QWidget* get_ui_parent_widget() const noexcept { return this->ui_parent; }

      QDialog* try_spawn_script_window() noexcept;
      void set_up_new_scripted_widget(QWidget*);  // Lua functions that create widgets must call this
      void accept_new_orphaned_widget(QWidget*);  // Lua functions that orphan widgets from a window must call this
      void widget_no_longer_orphaned(QWidget*);   // Lua functions that insert widgets into a window must call this
      void widget_no_longer_referenced(QWidget*); // called by wrapper internals when a widget is unreferenced

      void model_observer_reference_gained(ObservableStandardItemModelObserver*); // called by userdata-interface internals when a new observer wrapper is created
      void model_observer_reference_lost(ObservableStandardItemModelObserver*);   // called by wrapper internals when an observer wrapper is unreferenced

      //
      // Lua APIs that remove and delete items from a ObservableStandardItemModel should call 
      // this function after the removal is complete and control has returned to the script 
      // thread, in order to zombify any extant wrappers for the removed items.
      //
      // When removing a single item, it may be tempting to try and zombify just that one 
      // item's wrapper, but you should be aware that removing a single item can invalidate 
      // multiple observers (and thus require zombifying multiple wrappers): for example, if 
      // you remove a cell from a table with only one column, then you are also removing a 
      // row, and if Lua has accessed that row, it will have a separate observer and wrapper. 
      // There's also just the possibility that I might screw up somewhere, in a way that 
      // would allow a cell to have multiple observers/wrappers, and y'know, we should handle 
      // that case gracefully too!
      //
      void zombify_all_invalid_model_observers();

      void queue_lua_function(int stack_pos, bool lock_ui_for_function); // made available for Lua APIs

      int push_to_lua(const QVariant&);
      QVariant variant_from_lua(int stack_pos);
      
   signals:
      void messageLogged(const QString&);
      void scriptStarted();
      void scriptEnded(bool error);
      void userClickedLink(const QString& url, QWidget* opener);
      //
   public slots:
      void abort();
      void runScript(const QString& code, const QString& name);
      void setUIParentWidget(QWidget*); // only allowed when a script is not running
      //
   protected slots:
      void mainThreadLoop();

   protected:
      // Installed on all widgets and windows owned by script. Allows us to block ALL interaction with 
      // scripted UI while a Lua event listener is running.
      virtual bool eventFilter(QObject* object, QEvent* event) override;
};

class DovahKitScriptVMMessenger {
   //
   // This is an interface to DovahKitScriptVM, provided for the benefit of the Lua API functions 
   // themselves; it facilitates sending messages from the script thread to the main thread.
   //
   protected:
      DovahKitScriptVMMessenger(DovahKitScriptVM& w) : vm(w) {}
   public:
      static DovahKitScriptVMMessenger& get() {
         static DovahKitScriptVMMessenger instance(DovahKitScriptVM::get());
         return instance;
      }
      
      DovahKitScriptVM& vm;
      
      //
      // Sends a cross-thread-task to the main thread to be executed. If the task indicates that it's 
      // blocking, then blocks the caller (i.e. the script thread) until the main thread acknowledges 
      // the message. If the script is aborted while a blocking message is in transit, then this 
      // function will call luaL_error the same way our debug hook does, to ensure that the message 
      // sender doesn't continue execution with the message potentially unacknowledged or otherwise 
      // in an invalid state.
      //
      void send_message(editor_script::cross_thread_task* m);
      
      inline bool is_aborted() const noexcept { return vm.aborted; }
      inline bool is_running() const noexcept { return vm.running; }
};

class DovahKitScriptVMUITaskConduit {
   protected:
      DovahKitScriptVMUITaskConduit(DovahKitScriptVM& w) : vm(w) {}
   public:
      static DovahKitScriptVMUITaskConduit& get() {
         static DovahKitScriptVMUITaskConduit instance(DovahKitScriptVM::get());
         return instance;
      }

      DovahKitScriptVM& vm;

      void send_message(editor_script::ui_read_task&);
      void send_message(editor_script::cross_thread_task&);

      inline bool is_aborted() const noexcept { return vm.aborted; }
      inline bool is_running() const noexcept { return vm.running; }
};

class DovahKitScriptVMPermissionInterface {
   protected:
      DovahKitScriptVMPermissionInterface(DovahKitScriptVM& w) : vm(w) {}
   public:
      static DovahKitScriptVMPermissionInterface& get() {
         static DovahKitScriptVMPermissionInterface instance(DovahKitScriptVM::get());
         return instance;
      }
      //
      DovahKitScriptVM& vm;

      static void verify_form_write_permissions();
      static void verify_ui_permissions();

      static bool check_ui_html_permissions();
};

class DovahKitScriptVMUserdataInterface {
   //
   // This is an interface to DovahKitScriptVM, provided for the benefit of our userdata internals.
   //
   protected:
      DovahKitScriptVMUserdataInterface(DovahKitScriptVM& w) : vm(w) {}
   public:
      static DovahKitScriptVMUserdataInterface& get() {
         static DovahKitScriptVMUserdataInterface instance(DovahKitScriptVM::get());
         return instance;
      }
      //
      DovahKitScriptVM& vm;

      //
      // Remove a wrapper's metatable, and then remove it from the wrapper storage table. Effectively 
      // "kills" the wrapper. The wrapper will be deleted later, when Lua garbage-collects it.
      //
      void remove(editor_script::wrapper&);

      //
      // Kills all wrappers for the given form and any of its parts.
      //
      void remove_form(dovah::form_stub&);

      void remove_model_observer(ObservableStandardItemModelObserver&);

      //
      // Check if Lua already has an identical copy of the passed-in wrapper;  if so, push that copy 
      // onto the Lua stack. Otherwise, copy the passed-in wrapper into Lua and push it onto the Lua 
      // stack.
      //
      int push(lua_State*, const editor_script::wrapper&, const char* metatable_name);
      template<typename mt> inline int push(const editor_script::wrapper& instance) {
         static_assert(std::is_base_of_v<editor_script::wrapper_metatable, mt>);
         static_assert(!std::is_same_v<editor_script::wrapper_metatable, mt>);
         //
         return this->push(this->vm.lua_vm, instance, mt::metatable_key);
      }

      void remove_from_sequential_collection(editor_script::wrapper& to_remove);
};

class DovahKitScriptUIListenerInterface {
   protected:
      DovahKitScriptUIListenerInterface(DovahKitScriptVM& w) : vm(w) {}
   public:
      static DovahKitScriptUIListenerInterface& get() {
         static DovahKitScriptUIListenerInterface instance(DovahKitScriptVM::get());
         return instance;
      }
      
      DovahKitScriptVM& vm;

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