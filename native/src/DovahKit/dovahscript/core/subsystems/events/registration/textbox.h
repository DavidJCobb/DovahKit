#pragma once
#include "../registration.h"

namespace dovahscript::impl::event_registration {
   class textbox : public base {
      public:
         static result register_event(QObject& object, const char* event_name, const char* listener_name);

         static constexpr const std::initializer_list<const char*> event_names = {
            "OnChanged",       // The textbox's value was previously altered, and the user hit Enter or moved focus away from the textbox.
            "OnInputRejected", // The textbox rejected input because it didn't validate or the max length would've been exceeded.
            "OnKeyPressed",    // The textbox's value was altered by a keypress.
         };
   };
}