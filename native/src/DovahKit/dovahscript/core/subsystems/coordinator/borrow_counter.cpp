#include "borrow_counter.h"

namespace dovahscript::impl {
   borrow_counter::value_type borrow_counter::operator++() noexcept {
      std::unique_lock guard(this->lock);
      return ++this->counter;
   }
   borrow_counter::value_type borrow_counter::operator--() noexcept {
      value_type out;
      {
         std::unique_lock guard(this->lock);
         out = --this->counter;
         if (out == 0)
            this->cvar.notify_all();
      }
      return out;
   }
}