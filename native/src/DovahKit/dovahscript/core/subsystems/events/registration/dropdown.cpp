#include "dropdown.h"
#include <QComboBox>
#include <QVariant>
#include "../../../helpers/qt/combobox.h"
#include "../../events.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result dropdown::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<QComboBox*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnChanged") == 0) {
         std::string ln = listener_name;
         core::subsystems::events::get()._connect_event(
            get_passkey(),
            QObject::connect(casted, QOverload<int>::of(&QComboBox::currentIndexChanged), &impl::get_event_connection_recipient(),
               [casted, ln](int index) {
                  auto* proxy   = casted->model();
                  int   logical = cobb::qt::map_combobox_index_from_proxy(casted, index); // map proxy combobox index to logical combobox index
                  ++logical; // Lua is one-indexed, not zero-indexed
                  core::subsystems::events::get().receive_event_from_main_thread(*casted, "OnChanged", ln.c_str(), { logical });
               }
            ),
            object, event_name, listener_name
         );
         return result::success;
      }
      return result::failure;
   }
}