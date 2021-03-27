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
#include "wrapper.h"

namespace dovah {
   class form_stub;
}

class DovahKitScriptVMMessenger;
class DovahKitScriptVMUITaskConduit;
class DovahKitScriptVMPermissionInterface;
class DovahKitScriptVMUserdataInterface;

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
   public:

      // Storage in the Lua registry for a cached copy of (string.format), which we place there 
      // when we start up the VM, to ensure that hardcoded functions that need that behavior can 
      // access it even if the Lua script tries to monkeypatch or replace its own copy.
      static constexpr char* string_format_registry_key   = "cached:string.format";

      // Storage in the Lua registry for functions that have been queued by the script to execute 
      // after all listeners have completed. If a user script needs to trigger some lengthy task  
      // in response to a UI event listener, it should queue that task to run outside of that UI 
      // event listener, so that the scripted UI isn't blocked from updating by the task.
      static constexpr char* queued_function_registry_key = "dovah.internals.deferred_execution_queue";

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
         void process();

         //
         // Suitable only for use by the sending thread.
         //
         void wait_until_empty();

         void clear();
      };
      
      void _setup_lua_vm();
      void _teardown_lua_vm(); // can only safely run on the main thread, since it tears down Qt objects now too

      void _run_queued_functions();
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
      QWidget* ui_parent = nullptr;
      //
      struct {
         _task_queue s2m; // script-to-main
         struct { // main-to-script
            _task_queue urgent; // urgent messages MUST NOT trigger Lua code to execute!
            _task_queue normal; // TODO: UI signals should feed into this queue to trigger Lua code, or we should make a separate queue to replace this
         } m2s;
      } task_queues;
      //
      struct {
         std::vector<QDialog*> windows;
         std::vector<QWidget*> orphans;
      } widgets;
      struct {
         _task_queue read;
         _task_queue write;
      } ui_queues;
      
   public:
      static DovahKitScriptVM& get() {
         static DovahKitScriptVM instance;
         return instance;
      }
      //
      lua_State*  lua_vm = nullptr;
      std::thread thread;
      QTimer      main_thread_tick_timer;
      
      inline bool is_aborted() const noexcept { return this->aborted; }
      inline bool is_running() const noexcept { return this->running; }

      inline QWidget* get_ui_parent_widget() const noexcept { return this->ui_parent; }

      QDialog* try_spawn_script_window() noexcept;
      void accept_new_orphaned_widget(QWidget*);
      void widget_no_longer_orphaned(QWidget*);
      void widget_no_longer_referenced(QWidget*);
      
   signals:
      void messageLogged(const QString&);
      void scriptStarted();
      void scriptEnded(bool error);
      //
   public slots:
      void abort();
      void runScript(const QString& code, const QString& name);
      void setUIParentWidget(QWidget*); // only allowed when a script is not running
      //
   protected slots:
      void mainThreadLoop();
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

      void fire_event(QWidget&, const char* event_name, const std::vector<QVariant> params);
};