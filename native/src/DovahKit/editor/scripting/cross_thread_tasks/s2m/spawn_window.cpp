#include "spawn_window.h"
#include "../../editor_script_core.h"
#include "../../../core.h"

namespace editor_script::tasks::s2m {
   /*virtual*/ void spawn_window::_exec_impl() /*override*/ {
      this->result = DovahKitScriptVM::get().try_spawn_script_window();
      if (!this->result)
         this->error = true;
   }
}