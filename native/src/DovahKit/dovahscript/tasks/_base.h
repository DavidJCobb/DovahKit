#pragma once

struct lua_State;

namespace dovahscript::tasks {
   class _base {
      public:
         virtual ~_base() {}

         volatile bool seen = false; // has this message been received and acknowledged by its recipient?
      protected:
         //
         // For blocking script-to-client tasks only, this bool indicates whether the 
         // task needs to run Lua code as an essential part of its functioning. If so, 
         // we'll temporarily set the client thread as the script thread. We can afford 
         // to do this because the task blocks its sender anyway.
         //
         bool _needs_lua_ownership = false;

      protected:
         virtual void _exec_impl() = 0;

      public:
         void execute();

         //
         // To send a non-blocking task, simply add it to the VM's task queue and then 
         // continue on. To send a blocking message, add it to the VM's task queue and 
         // then loop until its (seen) property is set to (true); then, delete the task 
         // on your own.
         // 
         // This function may be called multiple times and should always return a 
         // consistent value.
         //
         virtual bool is_blocking() const noexcept { return false; }

         inline bool needs_lua_ownership() const noexcept { return this->_needs_lua_ownership; }
         virtual void run_lua_before(lua_State* L) {}; // Runs on the sender thread
   };
}