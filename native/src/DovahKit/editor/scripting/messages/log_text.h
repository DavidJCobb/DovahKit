#pragma once
#include "../messages.h"
#include <QString>

namespace editor_script::messages {
   class log_text : public message {
      //
      // Message for sending text to the main window to be displayed in a script output pane.
      //
      public:
         log_text() : message(message_type::log_text) {}
         //
         QString text;
   };
}
