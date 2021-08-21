#include "button.h"
#include <QPushButton>
#include <QVariant>
#include "../../events.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result button::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<QPushButton*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnActivated") == 0) {
         core::subsystems::events::get()._connect_event(get_passkey(), *casted, &QPushButton::clicked, event_name, listener_name);
         return result::success;
      }
      if (_stricmp(event_name, "OnCheckStateChanged") == 0) {
         core::subsystems::events::get()._connect_event(get_passkey(), *casted, &QPushButton::toggled, event_name, listener_name);
         return result::success;
      }
      return result::failure;
   }
}