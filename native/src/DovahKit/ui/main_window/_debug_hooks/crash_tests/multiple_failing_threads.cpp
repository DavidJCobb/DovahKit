#include "./multiple_failing_threads.h"
#include <chrono>
#include <thread>
#include "editor/subsystems/crash_dumper/register_new_thread.h"

using namespace std::chrono_literals;

namespace DovahKitDebug::features::crash_tests {
   static void thread_handler_a() {
      dovahkit::subsystems::crash_dumper::register_new_thread();
      throw std::exception("Test exception, meant to be uncaught (thread A)");
   }
   static void thread_handler_b() {
      dovahkit::subsystems::crash_dumper::register_new_thread();
      std::this_thread::sleep_for(2000ms);
      throw std::exception("Test exception, meant to be uncaught (thread B)");
   }

   /*static*/ void multiple_failing_threads::execute(QWidget* from) {
      // B sleeps so spawn it first
      auto b = std::thread(&thread_handler_b);
      b.detach();

      // that way, when A faults, the crash handler doesn't suspend our thread before we can spawn B
      auto a = std::thread(&thread_handler_a);
      a.detach();
   }
}