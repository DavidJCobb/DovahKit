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
#include "messages.h"

class DovahKitScriptVM : public QObject {
   Q_OBJECT
   //
   protected:
      DovahKitScriptVM();
      ~DovahKitScriptVM();
      //
      void _setup_lua_vm();
      void _teardown_lua_vm();
      //
   public:
      void _send_message(editor_script::message*); // public because APIs we provide to Lua need to be able to access it :(
   protected:
      std::recursive_mutex exec_lock;
      std::atomic<bool> aborted = false; // main thread can set this to kill the script
      bool running = false;
      struct {
         std::vector<editor_script::message*> list;
         std::recursive_mutex lock;
      } message_queue;
      //
   public:
      static DovahKitScriptVM& get() {
         static DovahKitScriptVM instance;
         return instance;
      }
      //
      lua_State*  lua_vm = nullptr;
      std::thread thread;
      QTimer      main_thread_tick_timer;
      //
      inline bool is_aborted() const noexcept { return this->aborted; }
      inline bool is_running() const noexcept { return this->running; }
      void take_all_messages(std::function<void(editor_script::message*)>);
      void adopt_from_owner_thread(); // call only from the owner thread
      //
   signals:
      void messageLogged(const QString&);
      void scriptStarted();
      void scriptEnded(bool error);
      //
   public slots:
      void abort();
      void ownerThreadLoop();
      void runScript(const QString& code, const QString& name);
};