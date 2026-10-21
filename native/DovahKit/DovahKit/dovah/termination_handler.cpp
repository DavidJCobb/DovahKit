#include "./termination_handler.h"
#include <exception> // std::set_terminate

namespace dovah {
   /*static*/ termination_handler& termination_handler::get() {
      static termination_handler instance;
      return instance;
   }

   termination_handler::handler_type termination_handler::get_handler() const {
      handler_type v = nullptr;
      {
         std::lock_guard guard(this->_mutex);
         v = this->_handler;
      }
      return v;
   }
   void termination_handler::set_handler(handler_type v) {
      std::lock_guard guard(this->_mutex);
      this->_handler = v;
   }

   /*static*/ void termination_handler::update_this_thread() {
      auto h = get().get_handler();
      if (h)
         std::set_terminate(h);
   }
}