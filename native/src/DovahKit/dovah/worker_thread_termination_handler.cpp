#include "./worker_thread_termination_handler.h"
#include <exception> // std::set_terminate

namespace dovah {
   /*static*/ worker_thread_termination_handler& worker_thread_termination_handler::get() {
      static worker_thread_termination_handler instance;
      return instance;
   }

   worker_thread_termination_handler::handler_type worker_thread_termination_handler::get_handler() const {
      handler_type v = nullptr;
      {
         std::lock_guard guard(this->_mutex);
         v = this->_handler;
      }
      return v;
   }
   void worker_thread_termination_handler::set_handler(handler_type v) {
      std::lock_guard guard(this->_mutex);
      this->_handler = v;
   }

   /*static*/ void worker_thread_termination_handler::update_this_thread() {
      auto h = get().get_handler();
      if (h)
         std::set_terminate(h);
   }
}