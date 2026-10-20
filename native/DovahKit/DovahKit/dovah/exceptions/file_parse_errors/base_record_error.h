#pragma once
#include "../file_parse_error.h"
#include "dovah/tes/record_header.h"

namespace dovah::exceptions::file_parse_errors {
   class base_record_error : public file_parse_error {
      public:
         base_record_error(
            const tes::file&          file,
            size_t                    record_offset,
            const tes::record_header& record_header
         ) :
            file_parse_error(file),
            record_offset(record_offset),
            record_header(record_header)
         {}

      public:
         size_t             record_offset; // offset of the record header within the file as a whole
         tes::record_header record_header;
   };
}