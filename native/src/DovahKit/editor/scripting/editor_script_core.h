#pragma once
#include "../../../Lua/lua.hpp"
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
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
   friend class DovahKitScriptVMPermissionInterface;
   friend class DovahKitScriptVMUserdataInterface;
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
      };
      
      void _setup_lua_vm();
      void _teardown_lua_vm();

      void _script_thread_loop();

      //
      // Returns (true) if the Lua VM should be kept alive even after the script has finished 
      // executing. This would be the case if there are any script-spawned UI windows that are 
      // still open and visible.
      //
      bool _should_keep_running() const noexcept;
      
      std::recursive_mutex exec_lock;
      std::atomic<bool> aborted = false; // main thread can set this to kill the script
      bool     running   = false;
      QWidget* ui_parent = nullptr;
      //
      struct {
         _task_queue s2m; // script-to-main
         struct { // main-to-script
            _task_queue urgent;
            _task_queue normal;
         } m2s;
      } task_queues;
      
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
      //
      DovahKitScriptVM& vm;
      //
      void send_message(editor_script::cross_thread_task* m); // blocks until the main thread acknowledges the message, if (m->is_blocking()) returns true. if the script is aborted while a blocking message is being sent, calls luaL_error
      //
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