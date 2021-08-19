#include "task_reference_state_multi_checker.h"
#include "../lifetime.h"

namespace {
   using passkey_t = cobb::passkey<dovahscript::core::subsystems::lifetime, dovahscript::impl::task_reference_state_multi_checker>;
}

namespace dovahscript::impl {
   task_reference_state_multi_checker::task_reference_state_multi_checker() : lifetime_sys(core::subsystems::lifetime::get()) {
   }
   task_reference_state_multi_checker::~task_reference_state_multi_checker() {
      this->set_active_state(false);
   }
   void task_reference_state_multi_checker::set_active_state(bool b) {
      if (this->is_active == b)
         return;
      this->is_active = b;
      this->lifetime_sys.set_task_reference_lock_state(passkey_t(), b);
   }
   bool task_reference_state_multi_checker::is_task_referenced(ObservableStandardItemModelObserver& subject) const noexcept {
      return this->lifetime_sys.lockless_test_is_task_referenced(passkey_t(), subject);
   }
   bool task_reference_state_multi_checker::is_task_referenced(QObject& subject) const noexcept {
      return this->lifetime_sys.lockless_test_is_task_referenced(passkey_t(), subject);
   }
}