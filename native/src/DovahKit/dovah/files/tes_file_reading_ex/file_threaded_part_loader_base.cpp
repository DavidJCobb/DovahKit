#include "file_threaded_part_loader_base.h"

namespace dovah::tes_file_reading {
   /*static*/ void file_threaded_part_loader_base::_thread_handler(file_threaded_part_loader_base* instance) {
      instance->exec();
   }
   //
   void file_threaded_part_loader_base::start() {
      this->thread = std::thread(_thread_handler, this);
   }
   void file_threaded_part_loader_base::wait_for() {
      this->thread.join();
   }
   float file_threaded_part_loader_base::assess_progress() const noexcept {
      return (float)this->progress.current / (float)this->progress.maximum;
   }
}