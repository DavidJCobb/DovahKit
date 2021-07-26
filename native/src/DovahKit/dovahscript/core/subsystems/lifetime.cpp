#include "lifetime.h"

namespace dovahscript::core::subsystems {
   void lifetime::main_thread_handler() {
      {
         auto& plc   = this->pending_lifetime_checks;
         auto  guard = std::lock_guard(plc.lock);
         if (plc.opportunity_handle.is_ready()) {
            static_assert(false, "TODO: Perform lifetime checks on the queued objects.");
         } else {
            bool empty = plc.queues.objects.empty() && plc.queues.model_observers.empty();
            if (!empty) {
               plc.opportunity_handle.request();
            }
         }
      }

      static_assert(false, "TODO: Do we need anything else here?");
   }
}