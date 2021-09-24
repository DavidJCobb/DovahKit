#pragma once
#include "../registration.h"
#include <QGroupBox>

namespace dovahscript::impl::event_registration {
   class groupbox : public base {
      public:
         using target_type = QGroupBox;

         static result register_event(QObject& object, const char* event_name, const char* listener_name);

         static constexpr const std::initializer_list<const char*> event_names = {
            "OnToggled",
         };
   };
}