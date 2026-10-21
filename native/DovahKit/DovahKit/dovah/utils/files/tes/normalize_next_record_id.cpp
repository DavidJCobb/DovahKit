#include "./normalize_next_record_id.h"

namespace dovah::utils::files::tes {
   extern bare_form_id normalize_next_record_id(bare_form_id id, file_prefix prefix) {
      id = prefix.strip_prefix(id);
      id |= 0xFF000000;
      return id;
   }
}