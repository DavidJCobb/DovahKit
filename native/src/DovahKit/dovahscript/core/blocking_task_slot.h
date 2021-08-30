#pragma once
#include <atomic>
#include <mutex>

namespace dovahscript {
   namespace tasks {
      class _base;
   }
}

namespace dovahscript::core {
   class blocking_task_slot {
      public:
         using task_t = dovahscript::tasks::_base;
      protected:
         std::atomic<task_t*> task = nullptr;
         
      public:
         // Call only from the sending thread.
         void send(task_t&);

         // Call only from the receiving thread.
         void process();

         // Call only from the receiving thread.
         void discard();
   };
}