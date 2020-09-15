#include "load_task.h"
#include "../../dovah/core.h"
#include "../../dovah/files/file_load_order.h"

namespace DovahKitEditorInternals {
   void load_task::exec() {
      benchmark.begin();
      result = editor.load_order->load_queued_files();
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
