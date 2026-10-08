#include "./core.h"
#include <QCoreApplication>
#include <Windows.h> // ExitProcess
#include "dovah/worker_thread_termination_handler.h"
#include "./main_process_state.h"
#include "./monitor_process_state.h"

namespace dovahkit::subsystems::crash_dumper {
   core::core() {
      auto args = QCoreApplication::arguments();
      bool is_child_process = false;
      for (auto arg : args) {
         if (arg == "--crash-handler") {
            is_child_process = true;
            break;
         }
      }
      if (is_child_process) {
         monitor_process_state::get_or_create();
         //
         // above won't return until either we finish handling a crash, we fail to 
         // handle a crash, or there is no crash
         //
         ExitProcess(0);
      } else {
         main_process_state::get_or_create();

         const auto handler = main_process_state::get_terminate_handler({});
         std::set_terminate(handler);
         dovah::worker_thread_termination_handler::get().set_handler(handler);
      }
   }
   core::~core() {
   }

   void core::register_new_thread() {
      auto handler = main_process_state::get_terminate_handler({});
      std::set_terminate(handler);
   }
}
