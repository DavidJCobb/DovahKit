#include "spawn_window.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../../core.h"

namespace editor_script::tasks::s2m {
   /*virtual*/ void spawn_window::_exec_impl() /*override*/ {
      this->result = DovahKitScriptVMCore::get().try_spawn_script_window();
      if (auto* w = this->result) {
         w->setWindowFlags(w->windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
      } else {
         this->error = true;
      }
   }
}