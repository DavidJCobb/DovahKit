#include "./register_new_thread.h"
#include "./core.h"

namespace dovahkit::subsystems::crash_dumper {
   extern void register_new_thread() {
      core::get_or_create().register_new_thread();
   }
}