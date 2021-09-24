#pragma once
#include "../registration.h"
#include <QTabWidget>

namespace dovahscript::impl::event_registration {
   class tabbox : public base {
      public:
         using target_type = QTabWidget;

         static result register_event(QObject& object, const char* event_name, const char* listener_name);

         static constexpr const std::initializer_list<const char*> event_names = {
            "OnSelectionChanged",
         };
   };
}