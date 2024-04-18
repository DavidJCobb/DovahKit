#pragma once
#include "./base_warning.h"

namespace dovah {
   class form_stub;
}

namespace dovah::notices {
   //
   // Warnings emitted when saving a form's full data.
   //
   class base_form_save_warning : public base_warning {
      public:
         constexpr base_form_save_warning(form_stub& s) : subject(s) {}

         struct {
            size_t file_offset = 0; // how far into the file we wrote before hitting the warning
         } file_info;
         form_stub& subject; // the form being loaded
   };
}