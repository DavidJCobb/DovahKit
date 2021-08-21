#include "checkbox.h"
#include <QCheckBox>
#include <QVariant>
#include "../../events.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result checkbox::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<QCheckBox*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnChanged") == 0) {
         std::string ln = listener_name;
         core::subsystems::events::get()._connect_event(
            get_passkey(),
            QObject::connect(casted, &QCheckBox::stateChanged, &impl::get_event_connection_recipient(),
               [casted, ln](int state) {
                  QString s;
                  switch (state) {
                     case Qt::CheckState::Checked:
                        s = "checked";
                        break;
                     case Qt::CheckState::PartiallyChecked:
                        s = "indeterminate";
                        break;
                     case Qt::CheckState::Unchecked:
                        s = "unchecked";
                        break;
                  }
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
            QObject::connect(casted, &QCheckBox::stateChanged, &impl::get_event_connection_recipient(),
               [casted, ln](int state) {
                  core::subsystems::events::get().receive_event_from_main_thread(*casted, "OnToggled", ln.c_str(), { state == Qt::CheckState::Checked });
               }
            ),
            object, event_name, listener_name
         );
         return result::success;
      }
      return result::failure;
   }
}