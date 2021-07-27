#pragma once
#include "../registration.h"

namespace dovahscript::impl::event_registration {
   class button : public base {
      public:
         static result register_event(QObject& object, const char* event_name, const char* listener_name);

         static constexpr const std::initializer_list<const char*> event_names = {
            "OnActivated",         // The button was clicked (or interacted with analogously via another input device).
            "OnCheckStateChanged", // The button is checkable and its check state changed.
         };
   };
}