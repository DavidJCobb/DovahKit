#pragma once
#include "./base_file_read_exception.h"

namespace dovah::tes {
   // The file structure is invalid somehow, and the problem was detected 
   // while working at the level of individual records.
   class structural_error_involving_record : public base_file_read_exception {
      public:
         structural_error_involving_record(
            const file& file,
            size_t      record_header_offset
         ) :
            base_file_read_exception(file),
            record_header_offset(record_header_offset)
         {}

      public:
         size_t record_header_offset; // within the containing file
   };
}