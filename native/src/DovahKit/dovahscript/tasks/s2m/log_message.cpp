#include "log_message.h"
#include "../../dovahscript_host.h"

namespace dovahscript::tasks::s2m {
   void log_message::_exec_impl() {
      emit host::get().messageLogged(this->text);
   }
}