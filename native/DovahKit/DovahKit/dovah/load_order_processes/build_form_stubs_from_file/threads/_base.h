#pragma once
#include <cstdint>
#include <exception>
#include <thread>
#include "dovah/exceptions/file_load_error.h"
#include "dovah/termination_handler.h"

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   template<typename Self>
   class _base {
      protected:
         std::thread _thread;
         struct {
            uint32_t current = 0;
            uint32_t maximum = 0;
         } _progress;
      public:
         std::exception_ptr exception;

      private:
         static void _thread_handler_impl(Self* instance) {
            termination_handler::update_this_thread();
            try {
               instance.execute_on_thread();
            } catch (const std::exception& ex) {
               instance->exception = std::current_exception();
            }
         }

      public:
         template<typename Self>
         void start(this Self& self) {
            self._thread = std::thread(&_thread_handler_impl, &self);
         }
         void wait_for() {
            if (!this->thread.joinable())
               return;
            this->thread.join();
         }

         constexpr float progress() const noexcept {
            return (float)this->_progress.current / this->_progress.maximum;
         }
   };
}