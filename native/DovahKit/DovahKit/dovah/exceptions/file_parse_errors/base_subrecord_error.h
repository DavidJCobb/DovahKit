#pragma once
#include "./base_record_error.h"

namespace dovah::exceptions::file_parse_errors {
   class base_subrecord_error : public base_record_error {
      public:
         base_subrecord_error(
            const tes::file&          file,
            size_t                    record_offset,
            const tes::record_header& record_header,
            size_t                    subrecord_offset
         ) :
            base_record_error(file, record_offset, record_header),
            subrecord_offset(subrecord_offset)
         {}

      public:
         size_t subrecord_offset = 0; // within decompressed record data
   };
}