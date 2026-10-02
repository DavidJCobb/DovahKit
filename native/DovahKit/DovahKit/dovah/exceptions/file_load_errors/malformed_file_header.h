#pragma once
#include <errno.h>
#include "../file_load_error.h"

namespace dovah::exceptions::file_load_errors {
   class malformed_file_header : public file_load_error {
      public:
         malformed_file_header(
            const std::filesystem::path& file,
            size_t offset
         ) :
            file_load_error(file),
            offset(offset)
         {}

      public:
         size_t offset = 0;
   };
}