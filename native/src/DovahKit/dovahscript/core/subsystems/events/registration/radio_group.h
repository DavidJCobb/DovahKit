#pragma once
#include "../registration.h"
#include <QButtonGroup>

namespace dovahscript::impl::event_registration {
   class radio_group : public base {
      public:
         using target_type = QButtonGroup;

         static result register_event(QObject& object, const char* event_name, const char* listener_name);

         static constexpr const std::initializer_list<const char*> event_names = {
            "OnSelectionChanged",
         };
   };
}