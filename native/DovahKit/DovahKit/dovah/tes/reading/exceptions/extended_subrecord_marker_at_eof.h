#pragma once
#include "./structural_error_involving_subrecord.h"

namespace dovah::tes {
   // For this exception type, `subrecord_header_offset` is the offset of the 'XXXX' subrecord header.
   class extended_subrecord_marker_at_eof : public structural_error_involving_subrecord {
      public:
         using structural_error_involving_subrecord::structural_error_involving_subrecord;
   };
}