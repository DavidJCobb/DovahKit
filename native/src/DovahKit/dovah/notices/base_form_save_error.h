#pragma once
#include "./base_error.h"

namespace dovah {
   class form_stub;
}

namespace dovah::notices {
   //
   // Errors emitted when attempting to save the full data for a form.
   //
   class base_form_save_error : public base_error {
      public:
         constexpr base_form_save_error(form_stub& s) : subject(s) {}

         struct {
            size_t file_offset = 0; // how far into the file we wrote before hitting the error
         } file_info;
         form_stub& subject; // the form being saved
   };
}