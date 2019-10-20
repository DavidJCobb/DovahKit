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
      // block allocator that splits blocks up by thread: a thread can identify itself 
      // to the allocator and be given a list of blocks; when the thread terminates, 
      // the list can later be given to another thread; and unrecognized threads just 
      // fall back to allocating on the normal heap via malloc/free. If I built that 
      // right, then allocations happening across multiple threads block each of them 
      // for a shorter amount of time: once the allocator has found the block list for 
      // the thread requesting an allocation, it can unlock.
      //
      // This can only work if a thread identifies itself to the allocator, and later 
      // (however directly or indirectly) notifies the allocator of its termination. 
      // This isn't possible with std::thread or (as far as I know) with std::async 
      // and std::future: the thread can't get a reference to the std::thread or 
      // std::future that represents it, and so can't pass that to the allocator. If 
      // the thread can't "pass itself" to the allocator, then the code that *created* 
      // the thread must pass the thread instead, and that introduces a race condition: 
      // the thread could try to allocate elements before it has been made known to the 
      // allocator.
      //
      // I'm not sure how that race condition would be handled. The allocator is built 
      // to fall back to malloc/free if an allocation takes place from an unrecognized 
      // thread (and if more threads try to register themselves with the allocator than 
      // the allocator can hold, then it simply ignores them, such that they're treated 
      // the same as unrecognized threads). I suspect that if a new thread tried to
      // allocate before being made known to the allocator, then it would just use that 
      // fallback rather than causing things to blow up. Still, why take chances?
      //
      // I'm not fully decided on all this yet, though, so here's a potential alternate 
      // approach that could utilize C++ STL stuff exclusively:
      //
      //  - The allocator has a member struct, thread_handle, which is created and 
      //    returned when the thread registers itself. This is similar to a smart 
      //    pointer or a lock guard; its destructor unregisters the containing thread. 
      //    The thread already has to call a function, of its own volition, to identify 
      //    itself to the allocator and be given a block list; the main difference here 
      //    is that we are actively informing the allocator of the thread's termination 
      //    rather than passively informing it (using a cobb::thread and marking it as 
      //    "dead" is passive).
      //
      //  - Threads are identified by std::thread::id, and the allocate function checks 
      //    std::this_thread::get_id(). We don't need to check whether a thread is alive 
      //    because it'll specifically unreigster itself when it terminates.
      //
      //  - We use std::async to spawn our async code. We don't need to bother with 
      //    std::future at all.
      //
      friend std::shared_ptr<thread> spawn_thread(thread_functor, void* state);
      public:
         HANDLE handle = INVALID_HANDLE_VALUE;
         DWORD  id     = 0;
         bool   alive  = false; // is the thread currently running?
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
         thread_result wait(uint32_t timeout_ms = infinite_thread_timeout); // block until this thread is complete
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