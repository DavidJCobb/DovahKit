#pragma once
#include <concepts>
#include <cstdint>
#include <exception>
#include <thread>
#include "dovah/termination_handler.h"
#include "../process_params.h"
namespace dovah {
   namespace callback_sets {
      struct data_load;
   }
   namespace load_order_processes::build_form_stubs_from_file {
      namespace passkeys {
         class accept_stub;
      }
      class process;
   }
   namespace tes {
      class file;
      class record_view;
   }
   class active_load_order;
   class form_stub;
}

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   struct initialization_params : public process_params {
      process& process;
   };

   template<typename Self>
   class _base {
      public:
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
         void wait_for() {
            if (!this->_thread.joinable())
               return;
            this->_thread.join();
         }

         constexpr float progress() const noexcept {
            return (float)this->_progress.current / this->_progress.maximum;
         }

      protected:
         template<
            std::same_as<process> Process = process,
            std::same_as<passkeys::accept_stub> Passkey = passkeys::accept_stub
         >
         void _finish_stub(
            form_stub*&&      stub,
            tes::record_view& record
         ) {
            Process& p = this->_config.process;
            p._done_parsing_stub(Passkey{}, stub, record);
         }
   };
}