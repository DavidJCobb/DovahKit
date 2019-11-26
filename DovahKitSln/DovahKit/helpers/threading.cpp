#include "threading.h"
#include <cassert>

namespace cobb {
   /*static*/ DWORD WINAPI thread::_handler(LPVOID data) {
      thread* t = (thread*)data;
      t->functor(t->state, *t);
      t->alive = false;
      return 0; // Windows thread return value for "no error;" causes an implicit call to ExitThread; does not implicitly free the handle
   }
   thread::~thread() {
      if (this->handle != INVALID_HANDLE_VALUE) {
         CloseHandle(this->handle);
         this->handle = INVALID_HANDLE_VALUE;
      }
      this->id = 0;
   }

   thread_result thread::wait(uint32_t timeout_ms) {
      if (!this->alive)
         return thread_result::already_closed;
      auto result = WaitForSingleObject(this->handle, timeout_ms);
      switch (result) {
         case 0:
            return thread_result::success;
         case 0x102:
            return thread_result::timeout;
         case 0xFFFFFFFF:
            return thread_result::failed;
      }
      return thread_result::unknown;
   }

   std::shared_ptr<thread> spawn_thread(thread_functor func, void* state) {
      auto t = std::make_shared<cobb::thread>();
      t->functor = func;
      t->state   = state;
      t->alive   = true;
      //
      DWORD  id;
      HANDLE handle = CreateThread(NULL, 0, &thread::_handler, t.get(), 0, &id);
      if (handle == NULL) {
         printf("Failed to create thread; last error was %d.", GetLastError());
         assert(false && "Failed to create thread.");
      }
      t->id     = id;
      t->handle = handle;
      return t;
   }
   thread_result wait_for_all_threads(uint32_t count, std::shared_ptr<thread> threads[], uint32_t timeout_ms) {
      assert(count && "This parameter cannot be zero.");
      assert(count <= MAXIMUM_WAIT_OBJECTS && "WinAPI limit on threads to wait for.");
      HANDLE handles[MAXIMUM_WAIT_OBJECTS];
      for (uint32_t i = 0; i < count; i++)
         handles[i] = threads[i]->handle;
      auto result = WaitForMultipleObjects(count, handles, true, timeout_ms);
      if (result >= 0 && result <= (WAIT_OBJECT_0 + count - 1))
         return thread_result::success;
      switch (result) {
         case 0x102:
            return thread_result::timeout;
         case 0xFFFFFFFF:
            return thread_result::failed;
      }
      return thread_result::unknown;
   }
   thread_result wait_for_any_threads(int32_t& out_first_index_to_finish, uint32_t count, std::shared_ptr<thread> threads[], uint32_t timeout_ms) {
      assert(count && "This parameter cannot be zero.");
      assert(count <= MAXIMUM_WAIT_OBJECTS && "WinAPI limit on threads to wait for.");
      HANDLE handles[MAXIMUM_WAIT_OBJECTS];
      for (uint32_t i = 0; i < count; i++)
         handles[i] = threads[i]->handle;
      auto result = WaitForMultipleObjects(count, handles, false, timeout_ms);
      if (result >= 0 && result <= (WAIT_OBJECT_0 + count - 1)) {
         out_first_index_to_finish = result - WAIT_OBJECT_0;
         return thread_result::success;
      }
      out_first_index_to_finish = -1;
      switch (result) {
         case 0x102:
            return thread_result::timeout;
         case 0xFFFFFFFF:
            return thread_result::failed;
      }
      return thread_result::unknown;
   }
}