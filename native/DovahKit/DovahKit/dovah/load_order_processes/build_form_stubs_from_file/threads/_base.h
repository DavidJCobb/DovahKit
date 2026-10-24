#pragma once
#include <cstdint>
#include <exception>
#include <thread>
#include "dovah/callback_sets/data_load.h"
#include "dovah/exceptions/file_load_error.h"
#include "dovah/termination_handler.h"
namespace dovah {
   namespace callback_sets {
      struct data_load;
   }
   namespace tes {
      class file;
   }
   class active_load_order;
}

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   template<typename Self>
   class _base {
      public:
         struct initialization_params {
            active_load_order&              active_load_order;
            const callback_sets::data_load& callbacks;
            const tes::file&                source_file;
         };

         _base(initialization_params v) : _config(v) {
         }

      protected:
         initialization_params _config;
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