#include "task_queue.h"
#include "../tasks/_base.h"

namespace {
   static constexpr bool use_atomic_waits = true;
}

namespace dovahscript::core {
   void task_queue::push_back(task_t* task) {
      auto  guard = std::lock_guard(this->lock);
      auto& list = this->list;
      list.push_back(task);
      this->is_empty = false;
   }

   void task_queue::process() {
      auto  guard = std::lock_guard(this->lock);
      auto& list  = this->list;
      //
      int count = 0;
      for (auto* task : list) {
         task->execute();
         delete task;
      }
      list.clear();
      this->is_empty = true;
      if constexpr (use_atomic_waits) {
         this->is_empty.notify_one();
      }
   }

   void task_queue::clear() {
      auto  guard = std::lock_guard(this->lock);
      auto& list = this->list;
      //
      for (auto* task : list)
         delete task;
      list.clear();
      this->is_empty = true;
      if constexpr (use_atomic_waits) {
         this->is_empty.notify_one();
      }
   }

   void task_queue::wait_until_empty() const noexcept {
      if constexpr (use_atomic_waits) {
         this->is_empty.wait(false); // wait until the variable is no longer false
      } else {
         while (!this->empty()) {}
      }
   }
}