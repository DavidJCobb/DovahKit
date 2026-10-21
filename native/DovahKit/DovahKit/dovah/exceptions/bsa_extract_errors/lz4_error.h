#pragma once
#include <cstdint>
#include "../bsa_extract_error.h"

namespace dovah::exceptions::bsa_extract_errors {
   class lz4_error : public bsa_extract_error {
      public:
         enum class failure_type {
            lz4_initialize,
            lz4_decompress,
         };

      public:
         lz4_error(
            const std::filesystem::path& archive_file,
            failure_type failure,
            size_t error_code
         ) :
            bsa_extract_error(archive_file),
            failure(failure),
            error_code(error_code)
         {}

      public:
         failure_type failure;
         size_t       error_code;
   };
}