#pragma once
#include "../messages.h"
#include <QString>

namespace editor_script::messages {
   class log_text : public message {
      public:
         log_text() : message(message_type::log_text) {}
         //
         QString text;
   };
}
