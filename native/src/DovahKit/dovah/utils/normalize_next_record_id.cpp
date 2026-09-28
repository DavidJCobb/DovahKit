#include "./normalize_next_record_id.h"

namespace dovah::utils {
   extern bare_form_id_t normalize_next_record_id(
      bare_form_id_t     next_form_id,
      const file_prefix& active_file_prefix
   ) {
      auto id = active_file_prefix.strip_prefix(next_form_id);
      id |= 0xFF000000;
      return id;
   }
}