#pragma once
#include <string>
#include "./base_error.h"

namespace dovah::notices {
   //
   // Errors emitted when initially loading files.
   //
   class base_file_load_error : public base_error {
      public:
         constexpr base_file_load_error() {}

         std::string filename;
         size_t      file_offset = 0;
   };
}