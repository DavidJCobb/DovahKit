#include "textbox.h"
#include <QLineEdit>
#include <QVariant>
#include "../../coordinator.h"
#include "../../events.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result textbox::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<QLineEdit*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnChanged") == 0) {
         std::string ln = listener_name;
         core::subsystems::events::get()._connect_event(
            get_passkey(),
            QObject::connect(casted, &QLineEdit::editingFinished, &impl::get_event_connection_recipient(),
               [casted, ln]() {
                  core::subsystems::events::get().receive_event_from_main_thread(*casted, "OnChanged", ln.c_str(), { casted->text() });
               }
            ),
            object, event_name, listener_name
         );
         return result::success;
      }
      if (_stricmp(event_name, "OnInputRejected") == 0) {
         core::subsystems::events::get()._connect_event(get_passkey(), *casted, &QLineEdit::inputRejected, event_name, listener_name);
         return result::success;
      }
      if (_stricmp(event_name, "OnKeyPressed") == 0) {
         core::subsystems::events::get()._connect_event(get_passkey(), *casted, &QLineEdit::textEdited, event_name, listener_name);
         return result::success;
      }
      return result::failure;
   }
}