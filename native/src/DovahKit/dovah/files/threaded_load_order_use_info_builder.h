#pragma once
#include <thread>
#include <vector>
#include "tes_file_reading/file.h"

namespace dovah {
   class threaded_load_order_use_info_builder : public tes_file_reading::basic_reader {
      protected:
         std::vector<form_stub*> queue;
         std::thread thread;
         //
         static void _thread_handler(threaded_load_order_use_info_builder* instance) {
            instance->_execute();
         }
         void _execute();
      public:
         threaded_load_order_use_info_builder() : tes_file_reading::basic_reader(nullptr) {}
         //
         void add_to_queue(form_stub* stub) noexcept;
         void start() noexcept;
         void wait_for() noexcept;
         //
         inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
   };
}