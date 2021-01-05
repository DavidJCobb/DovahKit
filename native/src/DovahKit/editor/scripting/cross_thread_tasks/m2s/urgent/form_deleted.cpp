#include "form_deleted.h"
#include "../../../editor_script_core.h"

namespace editor_script::tasks::m2s {
   /*virtual*/ void form_deleted::_exec_impl() /*override*/ {
      assert(this->stub);
      DovahKitScriptVMUserdataInterface::get().remove_form(*this->stub);
   }
}