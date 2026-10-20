#pragma once
#include "./base_record_error.h"

namespace dovah::exceptions::file_parse_errors {
   class record_decompression_failed : public base_record_error {
      public:
         enum class failure_type {
            claimed_size_is_too_huge_to_even_try,
            zlib_unknown_error,
            data_larger_than_expected,
            zlib_memory_error,
            data_corrupt_or_incomplete,
            uncompressed_data_is_not_of_declared_size,
         };

      public:
         record_decompression_failed(
            const tes::file& file,
            size_t record_offset,
            const tes::record_header& record_header,
            failure_type ft
         ) :
            base_record_error(file, record_offset, record_header),
            failure(ft)
         {}

      public:
         failure_type failure;
   };
}