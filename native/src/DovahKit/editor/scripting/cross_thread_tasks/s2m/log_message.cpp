#include "log_message.h"
#include "../../editor_script_core.h"

namespace editor_script::tasks::s2m {
   /*virtual*/ void log_message::_exec_impl() /*override*/ {
      auto& vm = DovahKitScriptVM::get();
      emit vm.messageLogged(this->text);
   }
}