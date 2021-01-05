#include "base.h"

namespace editor_script {
   void cross_thread_task::execute() {
      this->_exec_impl();
      this->seen = true;
   }
}