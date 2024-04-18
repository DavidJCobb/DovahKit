#pragma once
#include <string>
#include "./base_warning.h"

namespace dovah {
   class form_stub;
}

namespace dovah::notices {
   //
   // Warnings emitted when loading a form's full data on-demand.
   //
   class base_form_load_warning : public base_warning {
      public:
         constexpr base_form_load_warning(form_stub& s) : subject(s) {}

         struct {
            bool        is_winning_record = false;
            std::string source_file;
         } record_info;
         form_stub& subject; // the form being loaded
   };
}