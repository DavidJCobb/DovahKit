#pragma once
#include "lu/integral_character_code.h"
#include "./structural_error_involving_subrecord.h"

namespace dovah::tes {
   class truncated_subrecord : public structural_error_involving_subrecord {
      public:
         truncated_subrecord(
            const file& file,
            size_t      record_header_offset,
            size_t      subrecord_header_offset,
            lu::four_cc subrecord_signature,
            uint32_t    expected_size,
            uint32_t    truncated_size
         ) :
            structural_error_involving_subrecord(file, record_header_offset, subrecord_header_offset),
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