#include "task_queue.h"
#include "../tasks/_base.h"

namespace dovahscript::core {
   void task_queue::push_back(task_t* task) {
      auto  guard = std::lock_guard(this->lock);
      auto& list  = this->list;
      list.push_back(task);
      this->is_empty = false;
   }

   void task_queue::process() {
      auto  guard = std::lock_guard(this->lock);
      auto& list  = this->list;
      //
      int count = 0;
      for (auto* task : list) {
         bool blocking = task->is_blocking();
         task->execute();
         if (!blocking && task->is_fire_and_forget())
            delete task;
      }
      list.clear();
      this->is_empty = true;
   }

   void task_queue::clear() {
      auto  guard = std::lock_guard(this->lock);
      auto& list = this->list;
      //
      for (auto* task : list)
         if (!task->is_blocking() && task->is_fire_and_forget())
            delete task;
      list.clear();
      this->is_empty = true;
   }

   void task_queue::wait_until_empty() const noexcept {
      while (!this->empty()) {}
   }
}