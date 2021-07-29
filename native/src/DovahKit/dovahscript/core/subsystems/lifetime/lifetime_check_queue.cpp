#include "lifetime_check_queue.h"
#include "../lifetime.h"

namespace dovahscript::impl {
   bool lifetime_check_queue::_empty() const noexcept {
      if (this->queues.model_observers.empty())
         return true;
      if (this->queues.objects.empty())
         return true;
      return false;
   }

   void lifetime_check_queue::queue_check(model_observer_t& target) {
      auto  guard = std::lock_guard(this->lock);
      auto& list  = this->queues.model_observers;
      list.push_back(&target);
   }
   void lifetime_check_queue::queue_check(QObject& target) {
      auto  guard = std::lock_guard(this->lock);
      auto& list  = this->queues.objects;
      list.push_back(&target);
   }

   void lifetime_check_queue::main_thread_handler(subsystem_passkey) {
      auto guard = std::lock_guard(this->lock);
      if (!this->opportunity_handle.is_active()) {
         if (!this->_empty())
            this->opportunity_handle.request();
      }
      if (!this->opportunity_handle.is_ready())
         return;
      //
      static_assert(false, "TODO: Perform lifetime checks on the queued objects.");
   }
   void lifetime_check_queue::on_script_teardown(subsystem_passkey) {
      auto guard = std::lock_guard(this->lock);
      //
      this->queues.model_observers.clear();
      this->queues.objects.clear();
      //
      this->opportunity_handle.release();
   }
}