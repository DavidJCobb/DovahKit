#pragma once
#include <thread>
#include "basic_reader.h"

namespace dovah {
   class form_stub;
}

namespace dovah::tes_file_reading {
   class threaded_load_order_use_info_builder : basic_reader {
      protected:
         std::vector<form_stub*> queue;
         std::thread thread;
         struct {
            uint32_t maximum = 0;
            uint32_t current = 0;
         } progress;
         //
         static void _thread_handler(threaded_load_order_use_info_builder* instance) {
            instance->_execute();
         }
         void _execute();
      public:
         threaded_load_order_use_info_builder() : tes_file_reading::basic_reader() {}
         //
         void add_to_queue(form_stub* stub) noexcept;
         void start() noexcept;
         void wait_for() noexcept;
         inline void reserve(size_t s) {
            this->queue.reserve(s);
         }
         //
         inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
         float assess_load_progress() const noexcept;
   };
}