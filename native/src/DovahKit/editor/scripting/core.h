#pragma once
#include "../../../Lua/lua.hpp"
#include <atomic>
#include <thread>

class DovahKitScriptVM {
   protected:
      DovahKitScriptVM();
      ~DovahKitScriptVM();
      //
      void _setup_lua_vm();
      void _teardown_lua_vm();
      //
   public:
      static DovahKitScriptVM& get() {
         static DovahKitScriptVM instance;
         return instance;
      }
      //
      lua_State*  lua_vm = nullptr;
      std::thread thread;
      std::atomic<bool> aborted = false; // main thread can set this to kill the script
};