#pragma once
#include "../../../helpers/singleton.h"

namespace editor_script {
   class cross_thread_task;
   class ui_read_task;
}

class DovahKitScriptVMMessenger : cobb::singleton {
   //
   // This is an interface to DovahKitScriptVM, provided for the benefit of the Lua API functions 
   // themselves; it facilitates sending messages from the script thread to the main thread.
   //
   protected:
      DovahKitScriptVMMessenger() {}
   public:
      static DovahKitScriptVMMessenger& get() {
         static DovahKitScriptVMMessenger instance;
         return instance;
      }
      
      //
      // Sends a cross-thread-task to the main thread to be executed. If the task indicates that it's 
      // blocking, then blocks the caller (i.e. the script thread) until the main thread acknowledges 
      // the message. If the script is aborted while a blocking message is in transit, then this 
      // function will call luaL_error the same way our debug hook does, to ensure that the message 
      // sender doesn't continue execution with the message potentially unacknowledged or otherwise 
      // in an invalid state.
      //
      void send_message(editor_script::cross_thread_task* m);
};

class DovahKitScriptVMUITaskConduit : cobb::singleton {
   protected:
      DovahKitScriptVMUITaskConduit() {}
   public:
      static DovahKitScriptVMUITaskConduit& get() {
         static DovahKitScriptVMUITaskConduit instance;
         return instance;
      }

      void send_message(editor_script::ui_read_task&);
      void send_message(editor_script::cross_thread_task&);
};