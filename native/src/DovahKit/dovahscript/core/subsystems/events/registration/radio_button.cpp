#include "radio_button.h"
#include <QVariant>
#include "../../events.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result radio_button::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<target_type*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnChanged") == 0) {
         std::string ln = listener_name;
         core::subsystems::events::get()._connect_event(
            get_passkey(),
            QObject::connect(casted, &target_type::toggled, &impl::get_event_connection_recipient(),
               [casted, ln](bool checked) {
                  QString s = checked ? "checked" : "unchecked";
                  core::subsystems::events::get().receive_event_from_main_thread(*casted, "OnChanged", ln.c_str(), { s });
               }
            ),
            object, event_name, listener_name
         );
         return result::success;
      }
      if (_stricmp(event_name, "OnToggled") == 0) {
         std::string ln = listener_name;
         core::subsystems::events::get()._connect_event(
            get_passkey(),
            QObject::connect(casted, &target_type::toggled, &impl::get_event_connection_recipient(),
               [casted, ln](bool checked) {
                  core::subsystems::events::get().receive_event_from_main_thread(*casted, "OnToggled", ln.c_str(), { checked });
               }
            ),
            object, event_name, listener_name
         );
         return result::success;
      }
      return result::failure;
   }
}