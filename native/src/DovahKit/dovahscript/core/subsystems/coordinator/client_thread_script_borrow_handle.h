#pragma once

namespace dovahscript::core {
   namespace subsystems {
      class coordinator;
   }
   
   //
   // There are some cases where a DovahScript subsystem may want to have the worker thread 
   // pause, and have the client thread become the script thread. For example, there may be 
   // operations that need to perform both tasks which can only occur on the script thread, 
   // and tasks which can only occur on the client thread, and while these operations could 
   // be split across both threads, it may be  easier and more efficient to do them on one.
   // 
   // This struct serves as a means of allowing the client thread to borrow "script thread" 
   // status. A subsystem can simply have an instance  of this struct as a member and, when 
   // it needs to run script-related code from the client thread, it can call the "request" 
   // member function on the handle. It should then  continue on with its execution. Within 
   // the main thread loop, the subsystem should check whether this handle is "ready," that 
   // is, whether the worker thread is waiting. When the handle is ready, the subsystem can 
   // then run its task and release the handle.
   //
   class client_thread_script_borrow_handle {
      protected:
         bool active = false;
      public:
         client_thread_script_borrow_handle();
         ~client_thread_script_borrow_handle();

         client_thread_script_borrow_handle& operator=(const client_thread_script_borrow_handle&) noexcept;
         client_thread_script_borrow_handle& operator=(client_thread_script_borrow_handle&&) noexcept;
         client_thread_script_borrow_handle(const client_thread_script_borrow_handle&);
         client_thread_script_borrow_handle(client_thread_script_borrow_handle&&);

         bool is_ready() const noexcept;
         inline bool is_active() const noexcept { return this->active; }

         void request();
         void release();
   };
}