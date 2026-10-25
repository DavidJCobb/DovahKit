#pragma once
#include <concepts>
#include <cstdint>
#include <exception>
#include <thread>
#include "dovah/termination_handler.h"
#include "../process_params.h"
namespace dovah {
   namespace load_order_processes::build_form_stubs_from_file {
      class process;
   }
   namespace tes {
      class record_view;
   }
   class form_stub;
}

namespace dovah::load_order_processes::build_form_stubs_from_file {
   namespace threads {
      struct initialization_params : public process_params {
         process& process;
      };

      template<typename Self>
      class _base;
   }

   class base_worker_thread {
      template<typename T> friend class threads::_base;
      public:
         base_worker_thread(threads::initialization_params v) : _config(v) {}

      protected:
         threads::initialization_params _config;
         std::thread _thread;
         struct {
            uint32_t current = 0;
            uint32_t maximum = 0;
         } _progress;
      public:
         std::exception_ptr exception;
         size_t exception_offset = 0;

      protected:
         void _finish_stub(form_stub*&& stub, tes::record_view& record);
         void _set_stub_parent(form_stub& stub, form_stub* parent);

      public:
         inline void wait_for() {
            if (!this->_thread.joinable())
               return;
            this->_thread.join();
         }
         constexpr float progress() const noexcept {
            return (float)this->_progress.current / this->_progress.maximum;
         }
   };
}
namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   template<typename Self>
   class _base : public base_worker_thread {
      public:
         _base(initialization_params v) : base_worker_thread(v) {
         }

      private:
         static void _thread_handler_impl(Self* instance) {
            termination_handler::update_this_thread();
            try {
               instance->execute_on_thread();
            } catch (const std::exception& ex) {
               instance->exception = std::current_exception();
            }
         }

      public:
         template<typename Self>
         void start(this Self& self) {
            self._thread = std::thread(&_thread_handler_impl, &self);
         }
   };
}