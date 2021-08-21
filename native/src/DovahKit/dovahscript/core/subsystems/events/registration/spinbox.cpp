#include "spinbox.h"
#include <QDoubleSpinBox>
#include <QVariant>
#include "../../events.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result spinbox::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<QDoubleSpinBox*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnChanged") == 0) {
         core::subsystems::events::get()._connect_event(get_passkey(), *casted, QOverload<double>::of(&QDoubleSpinBox::valueChanged), event_name, listener_name);
         return result::success;
      }
      return result::failure;
   }
}