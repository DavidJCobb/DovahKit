#pragma once
#include <string>
#include "./base_warning.h"

namespace dovah {
   class form_stub;
}

namespace dovah::notices {
   //
   // Warnings emitted when loading a file, including when initially indexing forms and GMSTs.
   //
   class base_file_load_warning : public base_warning {
      public:
         constexpr base_file_load_warning() {}

         std::string source_file;
   };
}