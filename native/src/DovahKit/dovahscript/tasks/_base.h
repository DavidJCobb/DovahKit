#pragma once
#include <atomic>

struct lua_State;

namespace dovahscript::tasks {
   class _base {
      public:
         virtual ~_base() {}

         std::atomic<bool> seen = false; // only used by blocking tasks: has this message been received and acknowledged by its recipient?
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
         void mark_as_seen();

         virtual bool is_blocking() const noexcept { return false; }

         inline bool needs_lua_ownership() const noexcept { return this->_needs_lua_ownership; }
         virtual void run_lua_before(lua_State* L) {}; // Runs on the sender thread
   };
}