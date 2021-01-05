#pragma once
#include "../../../messages.h"

namespace dovah {
   class form_stub;
}

namespace editor_script::messages {
   class form_deleted : public message {
      public:
         form_deleted() : message(message_type::delete_form) {}
         //
         dovah::form_stub* stub = nullptr;
   };
}
