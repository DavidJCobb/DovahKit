#pragma once
#include "./base_file_read_exception.h"

namespace dovah::tes {
   class record_decompression_failed : public base_file_read_exception {
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
            const file&  file,
            size_t       offset,
            failure_type ft
         ) :
            base_file_read_exception(file),
            record_header_offset(offset),
            failure(ft)
         {}

      public:
         size_t       record_header_offset;
         failure_type failure;
   };
}