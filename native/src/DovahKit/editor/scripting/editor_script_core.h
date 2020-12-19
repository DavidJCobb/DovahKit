#pragma once
#include "../../../Lua/lua.hpp"
#include <atomic>
#include <mutex>
#include <thread>
#include <QObject>
#include <QString>

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
      std::recursive_mutex exec_lock;
      std::atomic<bool> aborted = false; // main thread can set this to kill the script
      bool running = false;
      //
   public:
      static DovahKitScriptVM& get() {
         static DovahKitScriptVM instance;
         return instance;
      }
      //
      lua_State*  lua_vm = nullptr;
      std::thread thread;
      //
      inline bool is_aborted() const noexcept { return this->aborted; }
      inline bool is_running() const noexcept { return this->running; }
      //
   signals:
      void scriptStarted();
      void scriptEnded(bool error);
      //
   public slots:
      void abort();
      void runScript(const QString& code, const QString& name);
};