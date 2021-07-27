#include "tabbox.h"
#include <QTabWidget>
#include <QVariant>
#include "../../coordinator.h"
#include "../../events.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result tabbox::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<QTabWidget*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnSelectionChanged") == 0) {
         std::string ln = listener_name;
         core::subsystems::events::get()._connect_event(
            get_passkey(),
            QObject::connect(casted, &QTabWidget::currentChanged, &core::subsystems::coordinator::get(),
               [casted, ln](int index) {
                  QWidget* widget = casted->widget(index);
                  core::subsystems::events::get().receive_event_from_main_thread(*casted, "OnSelectionChanged", ln.c_str(), { QVariant::fromValue<QObject*>(widget) });
               }
            ),
            object, event_name, listener_name
         );
         return result::success;
      }
      return result::failure;
   }
}