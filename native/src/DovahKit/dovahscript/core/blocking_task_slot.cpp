#include "blocking_task_slot.h"
#include <cassert>
#include "../tasks/_base.h"

namespace dovahscript::core {
   // Call only from the sending thread.
   void blocking_task_slot::send(task_t& task) {
      auto guard = std::unique_lock(this->lock);
      assert(!this->task);
      this->task = &task;
   }

   // Call only from the receiving thread.
   void blocking_task_slot::process() {
      task_t* task = nullptr;
      {
         auto guard = std::unique_lock(this->lock);
         task = this->task;
         this->task = nullptr;
      }
      if (task) {
         task->execute();
         task->mark_as_seen();
      }
   }

   // Call only from the receiving thread.
   void blocking_task_slot::discard() {
      task_t* task = nullptr;
      {
         auto guard = std::unique_lock(this->lock);
         task = this->task;
         this->task = nullptr;
      }
      if (task)
         task->mark_as_seen();
   }
}