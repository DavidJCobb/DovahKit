#pragma once
#include "./base_subrecord_error.h"

namespace dovah::exceptions::file_parse_errors {
   // For this exception type, `subrecord_offset` is the offset of the 'XXXX' subrecord header.
   class extended_subrecord_marker_at_eof : public base_subrecord_error {
      public:
         using base_subrecord_error::base_subrecord_error;
   };
}