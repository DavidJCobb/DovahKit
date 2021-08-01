#include "task_reference_state_multi_checker.h"
#include "../lifetime.h"

namespace {
   using passkey_t = cobb::passkey<dovahscript::core::subsystems::lifetime, dovahscript::impl::task_reference_state_multi_checker>;
}

namespace dovahscript::impl {
   task_reference_state_multi_checker::task_reference_state_multi_checker() : lifetime_sys(core::subsystems::lifetime::get()) {
      this->lifetime_sys.set_task_reference_lock_state(passkey_t(), true);
   }
   task_reference_state_multi_checker::~task_reference_state_multi_checker() {
      this->lifetime_sys.set_task_reference_lock_state(passkey_t(), false);
   }
   bool task_reference_state_multi_checker::is_task_referenced(ObservableStandardItemModelObserver& subject) const noexcept {
      return this->lifetime_sys.lockless_test_is_task_referenced(passkey_t(), subject);
   }
   bool task_reference_state_multi_checker::is_task_referenced(QObject& subject) const noexcept {
      return this->lifetime_sys.lockless_test_is_task_referenced(passkey_t(), subject);
   }
}