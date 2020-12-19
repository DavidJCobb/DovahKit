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
#include "messages.h"

class DovahKitScriptVMMessenger;

class DovahKitScriptVM : public QObject {
   Q_OBJECT
   //
   // This is the core singleton for editor scripting. It is intended to be accessed directly by 
   // the main thread, and will run Lua scripts on a secondary thread. The singleton has its own 
   // Qt signal for the main thread event loop, and  will loop in the secondary thread as needed 
   // to block script execution  while waiting for any needed  information from the main thread.
   //
   friend class DovahKitScriptVMMessenger;
   protected:
      DovahKitScriptVM();
      ~DovahKitScriptVM();
      
      void _setup_lua_vm();
      void _teardown_lua_vm();
      
      void _send_outbound_message(editor_script::message*);
      
      std::recursive_mutex exec_lock;
      std::atomic<bool> aborted = false; // main thread can set this to kill the script
      bool     running   = false;
      QWidget* ui_parent = nullptr;

      //
      // Lockable queue for messages outbound from the script thread to the main thread.
      //
      struct {
         std::vector<editor_script::message*> list;
         std::recursive_mutex lock;
      } outbound_message_queue;
      
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

      //
      // The main thread should use this function to view messages. The functor should return (true) 
      // to acknowledge a message, allowing that message to be removed from the outbound message 
      // queue.
      //
      void view_messages(std::function<bool(editor_script::message*)> functor);
      
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
      void send_message(editor_script::message* m); // blocks until the main thread acknowledges the message, if (m->is_blocking()) returns true
      //
      inline bool is_aborted() const noexcept { return vm.aborted; }
      inline bool is_running() const noexcept { return vm.running; }
};