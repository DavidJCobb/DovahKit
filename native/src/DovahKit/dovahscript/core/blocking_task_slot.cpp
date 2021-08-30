#include "blocking_task_slot.h"
#include <cassert>
#include "../tasks/_base.h"

namespace dovahscript::core {
   // Call only from the sending thread.
   void blocking_task_slot::send(task_t& task) {
      auto* prior = this->task.exchange(&task);
      assert(!prior && "How did the sender thread send one blocking task before we finished handling the last one it sent?!");
   }

   // Call only from the receiving thread.
   void blocking_task_slot::process() {
      auto* task = this->task.exchange(nullptr);
      if (task) {
         task->execute();
         task->mark_as_seen();
      }
   }

   // Call only from the receiving thread.
   void blocking_task_slot::discard() {
      auto* task = this->task.exchange(nullptr);
      if (task)
         task->mark_as_seen();
   }
}