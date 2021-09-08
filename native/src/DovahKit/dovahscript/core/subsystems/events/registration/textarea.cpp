#include "textarea.h"
#include "../../../../qt/DovahscriptTextarea.h"
#include <QVariant>
#include "../../events.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result textarea::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<DovahscriptTextarea*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnChanged") == 0) {
         core::subsystems::events::get()._connect_event(get_passkey(), *casted, &DovahscriptTextarea::inputAccepted, event_name, listener_name);
         return result::success;
      }
      if (_stricmp(event_name, "OnCommitted") == 0) {
         core::subsystems::events::get()._connect_event(get_passkey(), *casted, &DovahscriptTextarea::userSubmitted, event_name, listener_name);
         return result::success;
      }
      if (_stricmp(event_name, "OnInputRejected") == 0) {
         core::subsystems::events::get()._connect_event(get_passkey(), *casted, &DovahscriptTextarea::inputRejected, event_name, listener_name);
         return result::success;
      }
      return result::failure;
   }
}