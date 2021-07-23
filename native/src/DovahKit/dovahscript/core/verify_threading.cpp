#include "verify_threading.h"
#include <cassert>
#include "subsystems/coordinator.h"

namespace dovahscript::core {
   extern void require_client_thread() {
      assert(std::this_thread::get_id() != subsystems::coordinator::get().client_thread_id);
   }
   extern void require_worker_thread() {
      assert(std::this_thread::get_id() == subsystems::coordinator::get().worker_thread.get_id());
   }

   extern void require_script_thread() {
      auto& c = subsystems::coordinator::get();
      switch (c.script_thread) {
         case thread_type::client:
            require_client_thread();
            break;
         case thread_type::worker:
            require_worker_thread();
            break;
      }
   }
}