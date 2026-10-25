#pragma once
#include <cstdint>
#include <thread>
#include <vector>
namespace dovah {
   class active_load_order;
   class form_stub;
}

namespace dovah::load_order_processes::use_info_build {
   class worker_thread {
      protected:
         active_load_order& owner;
         std::vector<form_stub*> queue;
         std::thread thread;
         struct {
            uint32_t maximum = 0;
            uint32_t current = 0;
         } progress;
         
         static void _thread_handler(worker_thread* instance) {
            instance->_execute();
         }
         void _execute();

      public:
         worker_thread(active_load_order& owner) : owner(owner) {}
         
         void add_to_queue(form_stub* stub) noexcept;
         void start() noexcept;
         void wait_for() noexcept;
         inline void reserve(size_t s) {
            this->queue.reserve(s);
         }
         
         inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
         float assess_load_progress() const noexcept;

         struct {
            std::exception_ptr captured;
            size_t thrown_at_file_offset = 0;
         } exception;
   };
}