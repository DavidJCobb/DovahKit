#pragma once
#include "../registration.h"
#include "widgets/DKColorPickerButton.h"

namespace dovahscript::impl::event_registration {
   class color_button : public base {
      public:
         using target_type = DKColorPickerButton;

         static result register_event(QObject& object, const char* event_name, const char* listener_name);

         static constexpr const std::initializer_list<const char*> event_names = {
            "OnChanged",
         };
   };
}