#include "form_deleted.h"
#include "../../../systems/userdata.h"

namespace editor_script::tasks::m2s {
   /*virtual*/ void form_deleted::_exec_impl() /*override*/ {
      assert(this->stub);
      DovahKitScriptVMUserdataInterface::get().remove_form(*this->stub);
   }
}