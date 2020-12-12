#include "load_task.h"
#include "../../dovah/core.h"
#include "../../dovah/files/file_load_order.h"

namespace DovahKitEditorInternals {
   load_task::load_task(DovahKitCore& ed) : editor(ed) {
      qRegisterMetaType<DovahKitEditorInternals::multithreadable_load_results>();
   }
   void load_task::exec() {
      benchmark.begin();
      result = editor.load_order->load_queued_files(this->results);
      benchmark.end();
      if (result) {
         editor.loaded = true;
         //
         stats.milliseconds = benchmark.milliseconds();
         stats.microseconds = benchmark.microseconds() % 1000;
         stats.file_count   = editor.load_order->file_count();
         emit complete(stats);
      } else {
         emit failed();
      }
      emit ended();
   }
}
