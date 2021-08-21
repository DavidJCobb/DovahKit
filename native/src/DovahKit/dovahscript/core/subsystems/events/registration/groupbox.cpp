#include "groupbox.h"
#include <QGroupBox>
#include <QVariant>
#include "../../events.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result groupbox::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<QGroupBox*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnToggled") == 0) {
         core::subsystems::events::get()._connect_event(get_passkey(), *casted, &QGroupBox::toggled, event_name, listener_name);
         return result::success;
      }
      return result::failure;
   }
}