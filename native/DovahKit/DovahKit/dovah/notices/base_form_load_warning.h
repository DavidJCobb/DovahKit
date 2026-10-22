#pragma once
#include <string>
#include "./base_warning.h"
namespace dovah {
   class form_stub;
}

namespace dovah::notices {
   class base_form_load_warning : public base_warning {
      public:
         constexpr base_form_load_warning(form_stub& stub) : stub(stub) {}

      public:
         form_stub& stub;
         struct {
            bool        is_winning_record = false;
            std::string source_filename;
         } record_info;
   };
}
