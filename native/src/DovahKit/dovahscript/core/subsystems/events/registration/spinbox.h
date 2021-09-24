#pragma once
#include "../registration.h"
#include <QDoubleSpinBox>

namespace dovahscript::impl::event_registration {
   class spinbox : public base {
      public:
         using target_type = QDoubleSpinBox;

         static result register_event(QObject& object, const char* event_name, const char* listener_name);

         static constexpr const std::initializer_list<const char*> event_names = {
            "OnChanged", // The spinbox's value has been altered by the user. Fires instantly for increment/decrement buttons; for typing, works like the textbox OnChanged event.
         };
   };
}