#pragma once
#include <errno.h>
#include "../file_load_error.h"

namespace dovah::exceptions::file_load_errors {
   class filesystem_error : public file_load_error {
      public:
         filesystem_error(
            const std::filesystem::path& file,
            errno_t code
         ) :
            file_load_error(file),
            error_code(code)
         {}

      public:
         errno_t error_code = {};
   };
}