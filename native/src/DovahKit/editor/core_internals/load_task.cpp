#include "./load_task.h"
#include "dovah/files/file_load_order.h"
#include "dovah/exceptions/file_load_failed.h"
#include "dovah/exceptions/invalid_load_order.h"

namespace DovahKitEditorInternals {
   load_task::load_task(DovahKitCore& ed) : editor(ed) {
   }
   void load_task::exec() {
      benchmark.begin();
      try {
         editor.load_order->load_queued_files();
         benchmark.end();
         this->result = true;
      } catch (const dovah::exceptions::invalid_load_order& ex) {
         benchmark.end();

         this->exception = std::current_exception();
         emit failed();
      } catch (const dovah::exceptions::file_load_failed& ex) {
         benchmark.end();

         this->exception = std::current_exception();
         emit failed();
      }
      if (result) {
         editor.loaded = true;
         //
         stats.milliseconds = benchmark.milliseconds();
         stats.microseconds = benchmark.microseconds() % 1000;
         stats.file_count   = editor.load_order->file_count();
         emit complete(stats);
      }
      emit ended();
   }
}
