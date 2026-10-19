#pragma once
#include "lu/integral_character_code.h"
#include "./structural_error_involving_record.h"

namespace dovah::tes {
   class record_has_invalid_signature : public structural_error_involving_record {
      public:
         record_has_invalid_signature(
            const file&  file,
            size_t       record_header_offset,
            lu::four_cc  record_signature
         ) :
            structural_error_involving_record(file, record_header_offset),
            record_signature(record_signature)
         {}

      public:
         size_t      record_header_offset;
         lu::four_cc record_signature;
   };
}