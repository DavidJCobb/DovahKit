#pragma once
#include <atomic>
#include <mutex>
#include <vector>

namespace dovahscript {
   namespace tasks {
      class _base;
   }
}

namespace dovahscript::core {
   //
   // A queue suitable for receiving and executing cross-thread tasks.
   //
   class task_queue {
      using task_t = dovahscript::tasks::_base;
      protected:
         std::vector<task_t*> list;
         std::recursive_mutex lock;
         std::atomic<bool>    is_empty = false;
         
      public:
         // Call only from the sending thread.
         void push_back(task_t*);
         
         // Process all extant tasks. Call only from the receiving thread.
         void process();

         void clear();

         inline bool empty() volatile const noexcept { return this->is_empty; }

         void wait_until_empty() const noexcept;
   };
}