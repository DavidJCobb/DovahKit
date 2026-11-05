#pragma once
#include "../../types/bare_form_id.h"
namespace dovah {
   struct file_prefix;
}

namespace dovah::utils::form_ids {
   extern bare_form_id normalize_next_record_id(bare_form_id next_form_id, const file_prefix& active_file_prefix);
}