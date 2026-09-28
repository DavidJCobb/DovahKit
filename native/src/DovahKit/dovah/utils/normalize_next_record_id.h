#pragma once
#include "./file_prefix.h"

namespace dovah::utils {
   //
   // Given an unoccupied form ID, normalize it to be written into TES4/HEDR+0x08, 
   // the "Next Object ID" field, in a manner at least somewhat consistent with 
   // official content.
   //
   extern bare_form_id_t normalize_next_record_id(
      bare_form_id_t     next_form_id,
      const file_prefix& active_file_prefix
   );
}