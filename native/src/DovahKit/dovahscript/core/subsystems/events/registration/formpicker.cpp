#include "formpicker.h"
#include "../../events.h"

#include "../../../../../editor/form_stub_meta_type.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result formpicker::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<target_type*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnChanged") == 0) {
         core::subsystems::events::get()._connect_event(get_passkey(), *casted, &target_type::formChanged, event_name, listener_name);
         return result::success;
      }
      return result::failure;
   }
}