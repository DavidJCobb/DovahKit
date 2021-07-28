#pragma once

namespace dovahscript::core {
   // Assert that the caller is running on the client thread.
   extern void require_client_thread();

   // Assert that the caller is running on the worker thread.
   extern void require_worker_thread();

   // Assert that the caller is running on whichever thread currently owns the Lua state.
   // Do not confuse this with (require_worker_thread); sometimes the client thread can 
   // be in control of the Lua state, while performing engine-internal tasks.
   extern void require_script_thread();
}