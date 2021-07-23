#include "_base.h"

namespace dovahscript::tasks {
   void _base::execute() {
      this->_exec_impl();
      this->seen = true;
   }
}