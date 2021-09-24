#pragma once
#include "../registration.h"
#include <QCheckBox>

namespace dovahscript::impl::event_registration {
   class checkbox : public base {
      public:
         using target_type = QCheckBox;

         static result register_event(QObject& object, const char* event_name, const char* listener_name);

         static constexpr const std::initializer_list<const char*> event_names = {
            "OnChanged", // Argument is the checkbox state as a string ("checked", "indeterminate", "unchecked").
            "OnToggled", // The same as OnChanged, but the argument is a boolean indicating whether the checkbox is checked.
         };
   };
}