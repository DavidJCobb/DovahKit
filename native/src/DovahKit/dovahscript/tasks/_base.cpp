#include "_base.h"
#include <type_traits>

namespace dovahscript::tasks {
   static_assert(std::has_virtual_destructor_v<_base>, "The base class needs to have a virtual destructor so that subclasses destroy their members properly.");

   void _base::execute() {
      this->_exec_impl();
   }
   void _base::mark_as_seen() {
      this->seen = true;
      this->seen.notify_one();
   }
}