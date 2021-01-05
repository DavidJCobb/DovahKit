#pragma once
#include "../messages.h"

namespace dovah {
   class form_stub;
}

namespace editor_script::messages {
   class delete_form : public message {
      public:
         delete_form() : message(message_type::delete_form) {}
         //
         dovah::form_stub* stub = nullptr;
         // TODO: put a results struct here so the caller can see whether the operation succeeded
         //
         virtual bool is_blocking() const noexcept override { return true; }
   };
}
