#pragma once
#include <string>
#include "./base_warning.h"

namespace dovah::notices {
   class base_file_parse_warning : public base_warning {
      public:
         constexpr base_file_parse_warning(
            const std::string& source_filename
         ) :
            source_filename(source_filename)
         {}

      public:
         std::string source_filename;
   };
}
