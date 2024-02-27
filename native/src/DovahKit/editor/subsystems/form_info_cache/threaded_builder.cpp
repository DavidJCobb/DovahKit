#include "./threaded_builder.h"
#include "dovah/form_stub.h"

namespace dovahkit::subsystems::form_info_cache {
   void threaded_builder::_execute() {
      auto& list = this->queue;
      //
      this->progress.maximum = 0;
      for (auto& item : list)
         this->progress.maximum += item.stubs.size();
      //
      for (auto& item : list) {
         auto* handler = item.handler;
         for (auto* stub : item.stubs) {
            stub->do_custom_parse(this, handler);
            ++this->progress.current;
         }
      }
   }
   //
   void threaded_builder::add_to_queue(handler_t handler, dovah::form_stub* stub) noexcept {
      for (auto& item : this->queue) {
         if (item.handler == handler) {
            item.stubs.push_back(stub);
            return;
         }
      }
      auto& item = this->queue.emplace_back();
      item.handler = handler;
      item.stubs.push_back(stub);
   }
   void threaded_builder::start() noexcept {
      this->thread = std::thread(_thread_handler, this);
   }
   void threaded_builder::wait_for() noexcept {
      if (this->is_active())
         this->thread.join();
   }
   float threaded_builder::assess_load_progress() const noexcept {
      if (!this->progress.maximum)
         return 0.0F;
      return (float)this->progress.current / (float)this->progress.maximum;
   }
}