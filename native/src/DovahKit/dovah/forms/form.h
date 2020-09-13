#pragma once
#include "../core.h"

namespace dovah {
   class form_stub;
   namespace loaded_forms {
      class Form {
         public:
            const form_type_t formType;
            Form(form_type_t ft) : formType(ft) {};
            //
            form_stub* stub = nullptr;
            //
            const char* get_editor_id() const noexcept;
      };
   }
}