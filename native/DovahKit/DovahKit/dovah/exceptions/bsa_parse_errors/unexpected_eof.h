#pragma once
#include "../bsa_parse_error.h"

namespace dovah::exceptions::bsa_parse_errors {
   class unexpected_eof : public bsa_parse_error {
      public:
         unexpected_eof(
            const std::filesystem::path& archive_file,
            size_t archive_size
         ) :
            bsa_parse_error(archive_file),
            archive_size(archive_size)
         {}

      public:
         size_t archive_size = 0;
   };
}