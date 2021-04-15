#include "log_message.h"
#include "../../systems/editor_script_inner_core.h"

namespace editor_script::tasks::s2m {
   /*virtual*/ void log_message::_exec_impl() /*override*/ {
      auto& vm = DovahKitScriptVMCore::get();
      emit vm.messageLogged(this->text);
   }
}