#include "lifetime.h"

namespace dovahscript::core::subsystems {
   void lifetime::main_thread_handler() {
      {
         auto& plc = this->pending_lifetime_checks;
         if (plc.opportunity_handle.is_ready()) {
            auto guard = std::lock_guard(plc.lock);
            static_assert(false, "TODO: Perform lifetime checks on the queued objects.");
         }
      }

      static_assert(false, "TODO: Do we need anything else here?");
   }
}