#include "threaded_load_order_use_info_builder.h"
#include "../../form_stub.h"

namespace dovah::tes_file_reading {
   void threaded_load_order_use_info_builder::_execute() {
      //_DEBUGMSG("[dovah::threaded_load_order_use_info_builder] Thread %08X has started processing %d forms.", std::this_thread::get_id(), this->queue.size());
      #if BENCHMARK_LOAD_ORDER_USE_INFO_BUILD == 1
         struct timeb bench_start;
         struct timeb bench_last;
         struct timeb bench_current;
         ftime(&bench_last);
         bench_start = bench_last;
         uint32_t i = 0;
         std::thread::id threadID = std::this_thread::get_id();
      #endif
      auto& list = this->queue;
      this->progress.maximum = list.size();
      for (auto it = list.begin(); it != list.end(); ++it) {
         (*it)->build_outbound_refs(this);
         ++this->progress.current;
         #if BENCHMARK_LOAD_ORDER_USE_INFO_BUILD == 1
            ftime(&bench_current);
            auto diff = (uint32_t)(1000.0 * (bench_current.time - bench_last.time)) + (bench_current.millitm - bench_last.millitm);
            if (diff > 1000) {
               diff = (uint32_t)(1000.0 * (bench_current.time - bench_start.time)) + (bench_current.millitm - bench_start.millitm);
               _DEBUGMSG("[dovah::threaded_load_order_use_info_builder] Thread %08X: Time %d ms: processed %d forms.", threadID, diff, i);
            }
            bench_last = bench_current;
            i++;
         #endif
      }
      //_DEBUGMSG("[dovah::threaded_load_order_use_info_builder] Thread %08X has finished processing %d forms.", std::this_thread::get_id(), this->queue.size());
   }
   //
   void threaded_load_order_use_info_builder::add_to_queue(form_stub* stub) noexcept {
      this->queue.push_back(stub);
   }
   void threaded_load_order_use_info_builder::start() noexcept {
      this->thread = std::thread(threaded_load_order_use_info_builder::_thread_handler, this);
   }
   void threaded_load_order_use_info_builder::wait_for() noexcept {
      if (this->is_active())
         this->thread.join();
   }
   float threaded_load_order_use_info_builder::assess_load_progress() const noexcept {
      if (!this->progress.maximum)
         return 0.0F;
      return (float)this->progress.current / (float)this->progress.maximum;
   }
}