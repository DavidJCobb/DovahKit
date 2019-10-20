#pragma once
#include <memory>
#include <Windows.h>
#include "intrusive_windows_defines.h"

namespace cobb {
   enum class thread_result {
      failed,
      already_closed,
      timeout,
      success,
      unknown,
   };
   constexpr uint32_t infinite_thread_timeout = INFINITE;

   typedef void(*thread_functor)(void* state, cobb::thread& thread); // void my_function(void* state, cobb::thread& thread);
   class thread;

   // DO NOT TYPEDEF std::shared_ptr<thread>; doing so breaks std::swap which breaks std::shared_ptr::operator=
   // at least in MSVC 2019. isn't the STL great?

   class thread {
      //
      // This class should be accessed through shared_ptr only. It's an alternative to 
      // std::thread created to meet the following design goals:
      //
      //  - It must be possible to pass cobb::thread instances to other objects, which 
      //    should be able to check at any time whether the thread has finished running.
      //
      //  - It must be possible for the function that the thread is running to, itself, 
      //    access its cobb::thread instance.
      //
      // These requirements were influenced by my desire to create a multi-threaded 
      // block allocator that allows a thread to take ownership of an entire block. 
      // The thread must be able to pass "itself" to the allocator to effect this 
      // granting of ownership, and the allocator must later be able to tell whether 
      // the thread that owns a set of blocks is still running.
      //
      friend std::shared_ptr<thread> spawn_thread(thread_functor, void* state);
      public:
         HANDLE handle = INVALID_HANDLE_VALUE;
         DWORD  id     = 0;
         bool   alive  = false;
      protected:
         thread_functor functor = nullptr;
         void* state = nullptr;
         //
         // Intentionally private constructor. Use (spawn_thread) to make a cobb::thread.
         // If you need an array of them, make an array of shared_ptrs instead.
         //
         thread() {};
         //
      public:
         //
         thread_result wait(uint32_t timeout_ms = infinite_thread_timeout);
         //
         inline operator bool() { return this->alive; }
         inline bool operator==(const cobb::thread& other) { return this->id = other.id; }
         //
         ~thread();
         //
      protected:
         static DWORD WINAPI _handler(LPVOID);
   };
   std::shared_ptr<thread> spawn_thread(thread_functor, void* state);

   thread_result wait_for_all_threads(uint32_t count, std::shared_ptr<thread> threads[], uint32_t timeout_ms = infinite_thread_timeout);
   thread_result wait_for_any_threads(int32_t& out_first_index_to_finish, uint32_t count, std::shared_ptr<thread> threads[], uint32_t timeout_ms = infinite_thread_timeout);
}