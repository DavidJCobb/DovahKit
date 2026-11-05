#include "./normalize_next_record_id.h"
#include "../../types/file_prefix.h"

namespace dovah::utils::form_ids {
   extern bare_form_id normalize_next_record_id(bare_form_id next_form_id, const file_prefix& active_file_prefix) {
      auto id = active_file_prefix.strip_prefix(next_form_id);
      id |= 0xFF000000;
      return id;
   }
}