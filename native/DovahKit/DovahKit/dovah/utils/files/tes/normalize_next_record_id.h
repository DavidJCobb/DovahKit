#pragma once
#include "dovah/types/bare_form_id.h"
#include "dovah/types/file_prefix.h"

namespace dovah::utils::files::tes {
   extern bare_form_id normalize_next_record_id(bare_form_id, file_prefix);
}