#include "./uncaught_exception_on_thread.h"
#include <thread>
#include "editor/subsystems/crash_dumper/register_new_thread.h"

namespace DovahKitDebug::features::crash_tests {
   static void thread_handler() {
      dovahkit::subsystems::crash_dumper::register_new_thread();
      throw std::exception("Test exception, meant to be uncaught");
   }

   /*static*/ void uncaught_exception_on_thread::execute(QWidget* from) {
      auto t = std::thread(&thread_handler);
      t.detach();
   }
}