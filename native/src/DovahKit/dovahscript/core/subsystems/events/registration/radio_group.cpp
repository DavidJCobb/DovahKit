#include "radio_group.h"
#include <QAbstractButton>
#include <QButtonGroup>
#include <QVariant>
#include "../../coordinator.h"
#include "../../events.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result radio_group::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<QButtonGroup*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnChanged") == 0) {
         std::string ln = listener_name;
         core::subsystems::events::get()._connect_event(
            get_passkey(),
            QObject::connect(casted, QOverload<QAbstractButton*,bool>::of(&QButtonGroup::buttonToggled), &impl::get_event_connection_recipient(),
               [casted, ln](QAbstractButton* button, bool checked) {
                  if (!checked)
                     return;
                  core::subsystems::events::get().receive_event_from_main_thread(*casted, "OnSelectionChanged", ln.c_str(), { QVariant::fromValue<QObject*>(button) });
               }
            ),
            object, event_name, listener_name
         );
         return result::success;
      }
      return result::failure;
   }
}