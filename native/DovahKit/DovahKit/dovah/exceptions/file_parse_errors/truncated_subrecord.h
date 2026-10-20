#pragma once
#include "lu/integral_character_code.h"
#include "./base_subrecord_error.h"

namespace dovah::exceptions::file_parse_errors {
   class truncated_subrecord : public base_subrecord_error {
      public:
         truncated_subrecord(
            const tes::file&          file,
            size_t                    record_offset,
            const tes::record_header& record_header,
            size_t                    subrecord_offset,
            lu::four_cc               subrecord_signature,
            uint32_t                  expected_size,
            uint32_t                  truncated_size
         ) :
            base_subrecord_error(file, record_offset, record_header, subrecord_offset),
            subrecord_signature(subrecord_signature),
            expected_size(expected_size),
            truncated_size(truncated_size)
         {}

      public:
         lu::four_cc subrecord_signature;
         uint32_t    expected_size;
         uint32_t    truncated_size;
   };
}