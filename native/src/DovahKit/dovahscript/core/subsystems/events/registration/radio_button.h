#pragma once
#include "../registration.h"
#include <QRadioButton>

namespace dovahscript::impl::event_registration {
   class radio_button : public base {
      public:
         using target_type = QRadioButton;

         static result register_event(QObject& object, const char* event_name, const char* listener_name);

         static constexpr const std::initializer_list<const char*> event_names = {
            "OnChanged", // Argument is the button state as a string ("checked", "unchecked").
            "OnToggled", // The same as OnChanged, but the argument is a boolean indicating whether the button is checked.
         };
   };
}