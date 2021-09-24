#pragma once
#include "../registration.h"
#include "../../../../qt/DovahscriptTextarea.h"

namespace dovahscript::impl::event_registration {
   class textarea : public base {
      public:
         using target_type = DovahscriptTextarea;

         static result register_event(QObject& object, const char* event_name, const char* listener_name);

         static constexpr const std::initializer_list<const char*> event_names = {
            "OnChanged",       // The textarea's value was altered by a keypress or other action.
            "OnCommitted",     // The user hit Ctrl + Enter while the textarea had focus.
            "OnInputRejected", // The textarea rejected input because it didn't validate or the max length would've been exceeded.
         };
   };
}