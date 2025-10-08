
# Performance monitoring and reporting

Currently, the backend load process is mostly opaque to the frontend. The frontend measures how long the load takes, and displays this in the program status bar, but it can't get granular statistics or report detailed progress. This is annoying for debugging.

Loading consists of the following steps:

* Normalize load order
* Begin asynchronously loading BSAs
* Load files sequentially
* Build use info
* Reparent persistent refs
* Wait for BSA load to complete

As for reporting performance:

* I've modified `bsa_load_order` to store benchmarks on itself, but there's no way to ferry that to the frontend yet. We can track how long the entire BSA load takes, and how long we block on BSA load after everything else is done; a tracepoint in `file_load_order.cpp` prints this information to the terminal output during debugging.
* `DovahKitCore` takes its own measurement of the entire backend load process, representing it via the `DovahKitCore::file_load_stats` struct.
  * Since DKC can access the BSA-load-order, we could update this struct to show BSA loading stats, so we could report those after load, at least.

My dream goal is to be able to display a detailed progress bar for the full load process -- multiple process bars for when processes run side-by-side, actually -- including both backend tasks and frontend tasks (e.g. initial setup for the form info cache and the Papyrus subsystem).

Additionally, it'd be nice if the Log Window showed the full performance stats.