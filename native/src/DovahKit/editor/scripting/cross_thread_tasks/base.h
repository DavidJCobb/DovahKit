#pragma once
#include <string>

namespace editor_script {
   class cross_thread_task {
      //
      // Lua scripts are run in a separate thread from the main thread. This class is used to 
      // allow the main and script threads to issue commands to each other. It is functionally 
      // identical to a lambda, except it's possible for the sender-thread to check whether they 
      // have finished executing, to block until they have finished executing, and to check their 
      // results, all despite their being executed on the recipient-thread.
      //
      // These are made to function with the "task queues" stored on the script singleton. Lua 
      // functions should send them using the DovahKitScriptVMMessenger interface.
      //
      // Allocate these on the heap, please.
      //
      public:
         bool seen = false; // has this message been received and acknowledged by its recipient?
         //
      protected:
         virtual void _exec_impl() = 0;
      public:
         void execute();

         //
         // If (true), then the task object will be deleted automatically after the 
         // task is performed. You would want to have this return (false) if you intend 
         // for the sender to retain the task object and check its status at a later 
         // time (in which case the sender must delete the task object at some point 
         // after it has been flagged as "seen").
         // 
         // This function may be called multiple times and should always return a 
         // consistent value.
         //
         virtual bool is_fire_and_forget() const noexcept { return true; }

         //
         // To send a non-blocking task, simply add it to the VM's task queue and then 
         // continue on. To send a blocking message, add it to the VM's task queue and 
         // then loop until its (seen) property is set to (true); then, delete the task 
         // on your own.
         // 
         // This function may be called multiple times and should always return a 
         // consistent value.
         //
         // If (is_blocking) returns (true), then (is_fire_and_forget) is not checked 
         // and is treated as if it returned (false).
         //
         virtual bool is_blocking() const noexcept { return false; }
   };

   class ui_read_task : public cross_thread_task {
      public:
         virtual bool is_blocking() const noexcept final { return true; }
         virtual bool is_fire_and_forget() const noexcept final { return false; }
   };
}
