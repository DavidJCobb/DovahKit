#include "ui_event.h"
#include "../../editor_script_core.h"

namespace editor_script::tasks::m2s {
   /*virtual*/ void ui_event::_exec_impl() /*override*/ {
      DovahKitScriptUIListenerInterface::get().fire_event(this->widget, this->event_name.c_str(), this->params);
   }
}